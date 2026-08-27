# Vues de tableaux dans Arccore {#arcanedoc_core_types_array_views}

[TOC]

Ce document décrit les quatre types de vues de tableaux non propriétaires fournis par
le composant de base `arccore` et explique quand utiliser chacun d'eux.

- \arcane{ArrayView} / \arcane{ConstArrayView} — `arccore/base/ArrayView.h`.
Ce sont les **API Arcane originales** (années 2000). Elles sont utilisées de manière omniprésente dans l'API publique du framework Arcane, par exemple pour les variables de maillage (`ArrayView<double>` retourné par `CellVariable<double>::values()`), les lecteurs de maillage, les E/S, etc.
- \arcane{Span} / \arcane{SmallSpan} — `arccore/base/Span.h`

Vous pouvez également ajouter des déclarations avant pour ces classes en utilisant
le fichier d'en-tête `arccore/base/BaseTypes.h`.

---

## 1. Propriétés communes

Les quatre classes implémentent le même concept : une **vue légère et non propriétaire sur un bloc de mémoire contigu**, similaire à un tableau C avec une taille.

- **Pas de gestion de mémoire.** La vue ne stocke qu'un pointeur et une taille. La mémoire est possédée par un conteneur (\arcane{Array}, \arcane{UniqueArray}, \arcane{SharedArray}, \arcane{NumArray}) ou par un tampon brut. Une vue n'est valide que tant que le conteneur sous-jacent n'est pas réalloué.
- **Copies peu coûteuses.** Les constructeurs et les opérateurs d'affectation ne copient que le pointeur et la taille — jamais les données.
- **Contiguïté garantie.** Tous les éléments sont consécutifs en mémoire, de sorte qu'une vue peut être passée à des API C ou réinterprétée en tant que octets (\arcane{asBytes()}).
- **Vérification des limites optionnelle.** Lorsque arccore est compilé en mode de vérification (`ARCCORE_CHECK` défini, c'est-à-dire `ARCCORE_BUILD_MODE=Debug`/`Check`), l'accès hors limites via `operator[]`/`at()` lève une `IndexOutOfRangeException`.
- **Interopérabilité avec `std::array`.** Les quatre types peuvent être construits à partir de (et affectés à partir de) `std::array<T, N>`.
- **Accesseurs communs** (la signature exacte dépend du type) :

  | Membre | Description |
  |---|---|
  | `size()` / `length()` | Nombre d'éléments |
  | `empty()` | `size() == 0` |
  | `data()` / `unguardedBasePointer()` | Pointeur vers le premier élément |
  | `operator[]`, `operator()`, `item(i)`, `at(i)` | Accès aux éléments (vérifié en mode de vérification) |
  | `setItem(i, v)` / `setAt(i, v)` | Écriture d'éléments (vues modifiables uniquement) |
  | `begin()` / `end()` | Itérateurs — utilisables avec une boucle `for` basée sur une plage |
  | `subView()` / `subSpan()` / `subPart()` / `subspan()` | Sous-vue `[abegin, abegin+asize)`, tronquée à la taille de la vue |
  | `contains(v)` | Recherche linéaire |
  | `copy(other)` | Copie élément par élément à partir d'une vue compatible |
  | `fill(v)` | Définit tous les éléments à `v` |

\note **Note sur la dénomination.** La famille Span fournit à la fois `subspan()` (orthographe `std::span` de C++20) et `subSpan()`/`subPart()` (orthographe %Arcane) avec un comportement identique. L'ancienne orthographe `subView()` est **dépréciée** (`ARCCORE_DEPRECATED_REASON("Y2023: use subSpan() instead")`).

---

## 2. `ArrayView<T>` — la vue classique modifiable

La classe \arcane{ArrayView} est définie dans `arccore/base/ArrayView.h`.

```cpp
template <class T>
class ArrayView
{
  // ...
  Integer m_size; //!< Nombre d'éléments
  T*      m_ptr;  //!< Pointeur vers le tableau
};
```

- **Modifiable** : \arcane{ArrayView::operator[]()} retourne `T&`, \arcane{ArrayView::data()} retourne `T*`.
- **Le type de taille est `Integer`**, qui est `Int32`. Le nombre maximum d'éléments est donc d'environ 2,1 × 10⁹ (`Int32`).
- **Disposition** : deux membres de données (`Integer m_size; T* m_ptr;`) → 16 octets dans la construction par défaut.
- **Code hôte uniquement** — aucune annotation `ARCCORE_HOST_DEVICE`, il ne fait donc pas partie de la surface de l'API de l'accélérateur (GPU).

### API typique

```cpp
ArrayView<T>            subView(Integer abegin, Integer asize);
ConstArrayView<T>       constView() const;
ConstArrayView<T>       subConstView(Integer abegin, Integer asize) const;
ArrayView<T>            subViewInterval(Integer index, Integer nb_interval);
void                    setArray(const ArrayView<T>& v);  // remplace ptr+size
void                    copy(const U& other);
```

### Exemple

```cpp
Real t[5];
ArrayView<Real> a(t, 5);   // vue sur un tableau C (pas de copie)
a[2] = 5.0;                // modifiable
Real sum = 0.0;
for (Real v : a.subView(0, 3))  // les 3 premiers éléments
  sum += v;
```

---

## 3. `ConstArrayView<T>` — la vue classique en lecture seule

La classe \arcane{ConstArrayView} est définie dans `arccore/base/ArrayView.h`.

```cpp
template <class T>
class ConstArrayView
{
  // ...
  Integer           m_size; //!< Nombre d'éléments
  const T*          m_ptr;  //!< Pointeur vers le tableau (lecture seule)
};
```

- Identique à \arcane{ArrayView<T>} **sauf** qu'elle n'expose que des pointeurs `const T*`, des références constantes et des itérateurs constants. Les éléments ne peuvent pas être modifiés via la vue.
- **Conversion implicite depuis `ArrayView<T>`** (constructeur et affectation) — un "upcast" sûr qui ne copie que ptr+size :

  ```cpp
  ArrayView<Real>      v;
  ConstArrayView<Real> cv = v;   // OK, implicite
  ```
  
- Même sémantique de taille qu'avec \arcane{ArrayView} (`Integer`).
- C'est le type de retour des accesseurs constants dans l'API %Arcane (par exemple, les accesseurs de variables de maillage `const`).

---

## 4. `Span<T, Extent>` — la vue de style C++20, de taille Int64

La classe \arcane{Span} est définie dans `arccore/base/Span.h`.

```cpp
template <typename T, Int64 Extent = DynExtent>
class Span : public SpanImpl<T, Int64, Extent> { ... };
```

Elle est conçue pour être similaire à la classe `std::span` de C++20, avec des ajouts spécifiques à %Arcane :

- **Taille stockée en tant que `Int64`** — peut adresser des tableaux avec plus de 2³¹ éléments, contrairement à \arcane{ArrayView}.
- **Portée (Extent) à la compilation.** `Extent` est par défaut `DynExtent` (`-1`, taille connue à l'exécution). Si un `Extent` positif est donné, la taille est une constante de compilation :

  - le membre de taille n'est pas stocké (stockage `ExtentStorage` vide), de sorte qu'un span à portée fixe n'est que de **8 octets** (le pointeur) ;
  - le constructeur `Span(T* ptr)` (sans argument de taille) devient disponible (`requires(!IsDynamic)`);
  - en mode de vérification, la construction avec une taille d'exécution non correspondante lève une exception.
- **`ARCCORE_HOST_DEVICE`** sur tous les accesseurs — le type fait partie de l'**API de l'accélérateur** et peut être utilisé dans le code de périphérique CUDA/HIP/SYCL.
- **Constance via le type d'élément** (comme `std::span`) : `Span<T>` est modifiable, `Span<const T>` est en lecture seule. Il n'y a pas de classe séparée "ConstSpan".

### Conversions (toutes implicites, copies ptr+size)

| De | Vers | Condition |
|---|---|---|
| `ArrayView<X>` | `Span<X>` / `SmallSpan<X>` | — |
| `ConstArrayView<X>` | `Span<const X>` / `SmallSpan<const X>` | `T` doit être `const X` |
| `Span<X>` / `SmallSpan<X>` | `Span<const X>` / `SmallSpan<const X>` | `T` doit être `const X` |
| `std::array<X, N>` | n'importe quel span de `X` ou `const X` | — |

`view_type` mappe chaque span au type hérité : `Span<T>::view_type` est `ArrayView<T>`, `Span<const T>::view_type` est `ConstArrayView<T>`. Les fonctions utilitaires `smallView()` / `constSmallView()` effectuent cette conversion explicitement.

### API typique

```cpp
Int64   size() const;
Int64   sizeBytes() const;                    // taille * sizeof(T)
Span<T, DynExtent> subspan(Int64 abegin, Int64 asize) const;   // orthographe std::span
Span<T, DynExtent> subSpan(Int64 abegin, Int64 asize) const;   // orthographe Arcane
Span<T, DynExtent> subPart(Int64 abegin, Int64 asize) const;
Span<T, DynExtent> subSpanInterval(Int64 index, Int64 nb_interval) const;
ArrayView<T> smallView();                     // conversion vers le type hérité
```

### Exemple

```cpp
// Portée dynamique (par défaut)
Span<Real> s = ArrayView<Real>(ptr, 1000);
s.subspan(10, 20)[0] = 1.0;

// Portée fixe : taille connue à la compilation, pas de taille stockée
Span<Real, 3> v3{ &buffer[0] };   // nécessite un alignement de 8 octets pour Extent == 3
static_assert(sizeof(v3) == 8);

// Vue en lecture seule
Span<const Real> cs = s;
```

---

## 5. `SmallSpan<T, Extent>` — l'équivalent de taille Int32

La classe \arcane{SmallSpan} est définie dans `arccore/base/Span.h` (hérite de `SpanImpl<T, Int32, Extent>`).

```cpp
template <typename T, Int32 Extent = DynExtent>
class SmallSpan : public SpanImpl<T, Int32, Extent> { ... };
```

- **API et sémantiques identiques à `Span`**, sauf que la taille est stockée en tant que **`Int32`** (déclarations avant : `BaseTypes.h:65`).
- La documentation de la classe ajoute une contrainte : *le nombre d'octets associés à la vue (`sizeBytes()`) doit également tenir dans un `Int32`* — c'est-à-dire qu'un `SmallSpan` d'éléments de 1 octet ne peut pas dépasser environ 2 Go de données.
- Mêmes fonctionnalités que `Span` : portée fixe ou dynamique, `ARCCORE_HOST_DEVICE`, conversions de type `std::span`-like, orthographe `subView()` dépréciée, etc.
- **Aide pour choisir le type de span à partir d'un type de taille** :

  ```cpp
  template <typename T, typename SizeType>
  class SpanTypeFromSize;
  // SpanTypeFromSize<T, Int32>::SpanType == SmallSpan<T>
  // SpanTypeFromSize<T, Int64>::SpanType == Span<T>
  ```
  
  Ceci est utilisé, par exemple, par `asBytes()` pour retourner le type de span correspondant au type de taille source.

### Quand préférer `SmallSpan` à `Span`

- Les données sont suffisamment petites pour une taille `Int32` (le cas courant : listes d'entités de maillage, variables de maille/nœud, tampons).
- Interopérabilité avec l'API héritée : la taille de `ArrayView` est `Integer` (= `Int32` dans la construction par défaut), donc `ArrayView ↔ SmallSpan` fonctionne en round-trip sans changement de largeur.
- C'est le choix naturel pour les petits tampons à portée fixe (`SmallSpan<Real, 3>` pour un vecteur, `SmallSpan<Real, 9>` pour une matrice 3x3, etc.).

---

## 6. Tableau de comparaison

| | \arcane{ArrayView<T>} | \arcane{ConstArrayView<T>} | \arcane{SmallSpan<T, Extent>} | \arcane{Span<T, Extent>} |
|---|---|---|---|---|
| En-tête | `arccore/base/ArrayView.h` | `arccore/base/ArrayView.h` | `arccore/base/Span.h` | `arccore/base/Span.h` |
| Génération | Hérité (années 2000) | Hérité (années 2000) | Nouveau (ère C++20) | Nouveau (ère C++20) |
| Modifiable | Oui | Non | `T` non-const | `T` non-const |
| Type de taille | `Integer` (`Int32` par défaut, `Int64` avec `ARCCORE_64BIT`) | `Integer` | `Int32` | `Int64` |
| Max éléments (construction par défaut) | ~2,1 × 10⁹ | ~2,1 × 10⁹ | ~2,1 × 10⁹ (et `sizeBytes()` ≤ `Int32`) | ~9,2 × 10¹⁸ |
| Portée à la compilation | Non | Non | Oui (`Extent` paramètre) | Oui (`Extent` paramètre) |
| Membre de taille stocké | Toujours | Toujours | Seulement si dynamique | Seulement si dynamique |
| `ARCCORE_HOST_DEVICE` (GPU) | Non | Non | Oui | Oui |
| API de type `std::span` | Non | Non | Oui | Oui |
| Mécanisme de constance | Classe séparée | Classe séparée | `const` dans le type d'élément | `const` dans le type d'élément |
| Utilisation typique | API publique %Arcane héritée (variables de maillage, E/S) | Retour d'accesseurs constants | Nouveau code hôte/GPU, données de taille Int32 | Nouveau code hôte/GPU, données potentiellement énormes |

---

## 7. Fonctions utilitaires au niveau des octets (depuis `Span.h`)

Ces fonctions libres font le pont entre les vues et les octets bruts (utile pour la sérialisation et les opérations de style `memcpy`) :

```cpp
SmallSpan<const std::byte> asBytes(const ArrayView<T>& s);          // octets en lecture seule
Span<const std::byte>      asBytes(const SpanImpl<T, SizeType, E>& s);
SmallSpan<std::byte>       asWritableBytes(const ArrayView<T>& s);  // octets modifiables
Span<std::byte>            asWritableBytes(const SpanImpl<...>& s); // T non-const
Span<T>                    asSpan(Span<std::byte, E> bytes);        // octets -> typé
SmallSpan<T>               asSmallSpan(SmallSpan<std::byte, E> bytes);
Span<T, N>                 asSpan(std::array<T, N>& s);             // std::array -> span
SmallSpan<T, N>            asSmallSpan(std::array<T, N>& s);

void binaryWrite(std::ostream& ostr, const Span<const std::byte>& bytes);
void binaryRead (std::istream& istr, const Span<std::byte>& bytes);
```

\arcane{asBytes()} retourne le type de span (Small/Span) correspondant au type de taille de la source via `SpanTypeFromSize`.

---

## 8. Quel type devrais-je utiliser ?

1. **Appeler les API publiques %Arcane existantes** (variables de maillage, \arcane{ItemGroup}, services d'E/S, ...): utilisez ce que l'API prend ou retourne — généralement \arcane{ArrayView} / \arcane{ConstArrayView}.
2. **Écrire du nouveau code**, en particulier du code d'accélérateur/GPU: préférez la famille Span (`ARCCORE_HOST_DEVICE`).
   - Par défaut : **`SmallSpan<T>`** (couvre toutes les tailles de données de maillage courantes et fonctionne en round-trip sans perte avec `ArrayView`).
   - Utilisez **`Span<T>`** lorsque le nombre d'éléments ou la taille en octets peut dépasser les limites de `Int32`.
   - Utilisez une **portée fixe** (`SmallSpan<T, N>` / `Span<T, N>`) pour les tailles connues à la compilation (vecteurs, matrices, petits tampons fixes) — l'objet ne contient alors que le pointeur.
3. **Ne jamais** traiter une vue comme un propriétaire : ne supprimez/libérez pas son `data()` et ne maintenez pas la vue en vie plus longtemps que le conteneur auquel elle fait référence.
4. **Migration du code hérité** : remplacez `subView()` par `subSpan()`/`subPart()`; `subView()` compile toujours mais est déprécié.

### Exemple rapide : style hérité vs. nouveau

```cpp
// Style hérité (toujours requis par la plupart des API publiques Arcane)
void f(const ConstArrayView<Real>& values)
{
  for (Real v : values) { /* ... */ }
}

// Style nouveau (fonctionne sur l'hôte et le périphérique)
void g(SmallSpan<const Real> values)
{
  Real s = 0.0;
  for (Real v : values) s += v;
}

// Interopérabilité : conversion implicite, coût nul
ArrayView<Real> av;            // par ex. à partir d'une variable de maillage
g(av);                         // ArrayView -> SmallSpan<const Real>
f(SmallSpan<const Real>(av));  // SmallSpan -> ConstArrayView
```
____

<div class="section_buttons">
<span class="back_section_button">
\ref arcanedoc_core_types_numarray
</span>
<!-- <span class="next_section_button">
\ref arcanedoc_core_types_axl_caseoptions
</span> -->
</div>
