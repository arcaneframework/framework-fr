// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// Voir le fichier COPYRIGHT de niveau supérieur pour les détails.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* Real2x2.h                                                   (C) 2000-2026 */
/*                                                                           */
/* Matrice 2x2 de 'Real'.                                                    */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_REAL2X2_H
#define ARCCORE_BASE_REAL2X2_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/Real2.h"
#include "arccore/base/TypeEqual.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*!
 * \brief Structure POD pour un Real2x2.
 */
struct Real2x2POD
{
 public:

  Real2POD x;
  Real2POD y;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*!
 * \brief Classe gérant une matrice 2x2 de réels.
 *
 * La matrice comprend deux composantes \a x et \a y qui sont de type \b Real2. Par exemple:
 *
 * \code
 * Real2x2 matrix;
 * matrix.x.y = 2.;
 * matrix.y.y = 3.;
 * \endcode
 */
class ARCCORE_BASE_EXPORT Real2x2
{
 public:

  //! Construit la matrice nulle
  constexpr ARCCORE_HOST_DEVICE Real2x2()
  : x(Real2::null())
  , y(Real2::null())
  {}

  //! Construit le couple (ax,ay)
  constexpr ARCCORE_HOST_DEVICE Real2x2(Real2 ax, Real2 ay)
  : x(ax)
  , y(ay)
  {}

#if 0
  /*!
   * \brief Construit le couple ((ax,bx),(ay,by)).
   * \déprécié Utilisez Real2x2(Real2 a,Real2 b) à la place.
   */
  ARCCORE_DEPRECATED_116 Real2x2(Real ax, Real ay, Real bx, Real by)
  : x(ax, bx)
  , y(ay, by)
  {}
#endif

  //! Construit une copie identique à \a f
  Real2x2(const Real2x2& f) = default;

  //! Construit une copie identique à \a f
  constexpr ARCCORE_HOST_DEVICE explicit Real2x2(const Real2x2POD& f)
  : x(f.x)
  , y(f.y)
  {}

  //! Construit l'instance avec le triplet (v,v,v).
  constexpr ARCCORE_HOST_DEVICE explicit Real2x2(Real v)
  {
    x = y = v;
  }

  //! Construit le couple ((av[0], av[1]), (av[2], av[3]))
  constexpr ARCCORE_HOST_DEVICE explicit Real2x2(ConstArrayView<Real> av)
  : x(av[0], av[1])
  , y(av[2], av[3])
  {}

  //! Opérateur d'affectation de copie
  Real2x2& operator=(const Real2x2& f) = default;

  //! Affecte le couple (v,v,v) à l'instance.
  constexpr ARCCORE_HOST_DEVICE Real2x2& operator=(Real v)
  {
    x = y = v;
    return (*this);
  }

 public:

  Real2 x; //!< Première composante
  Real2 y; //!< Deuxième composante

 public:

  //! Construit la matrice nulle
  constexpr ARCCORE_HOST_DEVICE static Real2x2 null() { return Real2x2(); }

  //! Construit le couple ((ax,bx),(ay,by)).
  constexpr ARCCORE_HOST_DEVICE static Real2x2 fromColumns(Real ax, Real ay, Real bx, Real by)
  {
    return Real2x2(Real2(ax, bx), Real2(ay, by));
  }

  //! Construit le couple ((ax,bx),(ay,by)).
  constexpr ARCCORE_HOST_DEVICE static Real2x2 fromLines(Real ax, Real bx, Real ay, Real by)
  {
    return Real2x2(Real2(ax, bx), Real2(ay, by));
  }

 public:

  //! Retourne une copie du couple.
  constexpr ARCCORE_HOST_DEVICE Real2x2 copy() const { return (*this); }

  //! Réinitialise le couple avec les constructeurs par défaut.
  constexpr ARCCORE_HOST_DEVICE Real2x2& reset()
  {
    *this = null();
    return (*this);
  }

  //! Affecte le couple (ax,ay) à l'instance
  constexpr ARCCORE_HOST_DEVICE Real2x2& assign(Real2 ax, Real2 ay)
  {
    x = ax;
    y = ay;
    return (*this);
  }

  //! Copie le couple \a f
  constexpr ARCCORE_HOST_DEVICE Real2x2& assign(Real2x2 f)
  {
    x = f.x;
    y = f.y;
    return (*this);
  }

  //! Retourne une vue des quatre éléments de la matrice.
  //! [x.x, x.y, y.x, y.y]
  constexpr ARCCORE_HOST_DEVICE ArrayView<Real> view()
  {
    return { 4, &x.x };
  }

  //! Retourne une vue constante des quatre éléments de la matrice.
  //! [x.x, x.y, y.x, y.y]
  constexpr ARCCORE_HOST_DEVICE ConstArrayView<Real> constView() const
  {
    return { 4, &x.x };
  }

  /*!
   * \brief Lit la matrice depuis le flux \a i
   * La matrice est lue sous la forme de trois Real2.
   */
  std::istream& assign(std::istream& i);

  //! Écrit le couple dans le flux \a o de manière à être lisible par assign()
  std::ostream& print(std::ostream& o) const;

  //! Écrit le couple dans le flux \a o sous la forme (x,y,z)
  std::ostream& printXy(std::ostream& o) const;

  //! Ajoute \a b au couple
  constexpr ARCCORE_HOST_DEVICE Real2x2& add(Real2x2 b)
  {
    x += b.x;
    y += b.y;
    return (*this);
  }

  //! Soustrait \a b du couple
  constexpr ARCCORE_HOST_DEVICE Real2x2& sub(Real2x2 b)
  {
    x -= b.x;
    y -= b.y;
    return (*this);
  }

  //! Multiplie chaque composante du couple par la composante correspondante de \a b
  //Real2x2& mul(Real2x2 b) { x*=b.x; y*=b.y; return (*this); }

  //! Divise chaque composante du couple par la composante correspondante de \a b
  constexpr ARCCORE_HOST_DEVICE Real2x2& div(Real2x2 b)
  {
    x /= b.x;
    y /= b.y;
    return (*this);
  }

  //! Ajoute \a b à chaque composante du couple
  constexpr ARCCORE_HOST_DEVICE Real2x2& addSame(Real2 b)
  {
    x += b;
    y += b;
    return (*this);
  }

  //! Soustrait \a b à chaque composante du couple
  constexpr ARCCORE_HOST_DEVICE Real2x2& subSame(Real2 b)
  {
    x -= b;
    y -= b;
    return (*this);
  }

  //! Multiplie chaque composante du couple par \a b
  constexpr ARCCORE_HOST_DEVICE Real2x2& mulSame(Real2 b)
  {
    x *= b;
    y *= b;
    return (*this);
  }

  //! Divise chaque composante du couple par \a b
  constexpr ARCCORE_HOST_DEVICE Real2x2& divSame(Real2 b)
  {
    x /= b;
    y /= b;
    return (*this);
  }

  //! Ajoute \a b au couple.
  constexpr ARCCORE_HOST_DEVICE Real2x2& operator+=(Real2x2 b) { return add(b); }

  //! Soustrait \a b du couple
  constexpr ARCCORE_HOST_DEVICE Real2x2& operator-=(Real2x2 b) { return sub(b); }

  //! Multiplie chaque composante du couple par la composante correspondante de \a b
  //Real2x2& operator*=(Real2x2 b) { return mul(b); }

  //! Multiplie chaque composante de la matrice par le réel \a b
  constexpr ARCCORE_HOST_DEVICE void operator*=(Real b)
  {
    x *= b;
    y *= b;
  }

  //! Divise chaque composante du couple par la composante correspondante de \a b
  //Real2x2& operator/= (Real2x2 b) { return div(b); }

  //! Divise chaque composante de la matrice par le réel \a b
  constexpr ARCCORE_HOST_DEVICE void operator/=(Real b)
  {
    x /= b;
    y /= b;
  }

  //! Crée un couple égal à ce couple ajouté à \a b
  constexpr ARCCORE_HOST_DEVICE Real2x2 operator+(Real2x2 b) const { return Real2x2(x + b.x, y + b.y); }

  //! Crée un couple égal à ce couple moins \a b
  constexpr ARCCORE_HOST_DEVICE Real2x2 operator-(Real2x2 b) const { return Real2x2(x - b.x, y - b.y); }

  //! Crée un tenseur inverse du tenseur actuel
  constexpr ARCCORE_HOST_DEVICE Real2x2 operator-() const { return Real2x2(-x, -y); }

  /*!
   * \brief Compare l'instance actuelle composante par composante à \a b.
   *
   * \retval vrai si this.x==b.x et this.y==b.y.
   * \retval faux sinon.
   */
  constexpr ARCCORE_HOST_DEVICE bool operator==(Real2x2 b) const
  {
    return (x == b.x) && (y == b.y);
  }

  /*!
   * \brief Compare deux couples.
   * Pour la notion d'égalité, voir operator==()
   * \retval vrai si les deux couples sont différents,
   * \retval faux sinon.
   */
  constexpr ARCCORE_HOST_DEVICE bool operator!=(Real2x2 b) const
  {
    return !operator==(b);
  }

  /*!
   * \brief Accès en lecture seule à la ligne \a i-ème (entre 0 et 1 inclus) de l'instance.
   * \param i numéro de ligne à retourner
   */
  ARCCORE_HOST_DEVICE Real2 operator[](Integer i) const
  {
    ARCCORE_CHECK_AT(i, 2);
    return (&x)[i];
  }

  /*!
   * \brief Accès en lecture seule à la ligne \a i-ème (entre 0 et 1 inclus) de l'instance.
   * \param i numéro de ligne à retourner
   */
  ARCCORE_HOST_DEVICE Real2 operator()(Integer i) const
  {
    ARCCORE_CHECK_AT(i, 2);
    return (&x)[i];
  }

  /*!
   * \brief Accès en lecture seule à la ligne \a i-ème et à la colonne \a j-ème.
   * \param i numéro de ligne à retourner
   * \param j numéro de colonne à retourner
   */
  ARCCORE_HOST_DEVICE Real operator()(Integer i, Integer j) const
  {
    ARCCORE_CHECK_AT(i, 2);
    ARCCORE_CHECK_AT(j, 2);
    return (&x)[i][j];
  }

  /*!
   * \brief Accès à la ligne \a i-ème (entre 0 et 1 inclus) de l'instance.
   * \param i numéro de ligne à retourner
   */
  ARCCORE_HOST_DEVICE Real2& operator[](Integer i)
  {
    ARCCORE_CHECK_AT(i, 2);
    return (&x)[i];
  }

  /*!
   * \brief Accès à la ligne \a i-ème (entre 0 et 1 inclus) de l'instance.
   * \param i numéro de ligne à retourner
   */
  ARCCORE_HOST_DEVICE Real2& operator()(Integer i)
  {
    ARCCORE_CHECK_AT(i, 2);
    return (&x)[i];
  }

  /*!
   * \brief Accès à la ligne \a i-ème et à la colonne \a j-ème.
   * \param i numéro de ligne à retourner
   * \param j numéro de colonne à retourner
   */
  ARCCORE_HOST_DEVICE Real& operator()(Integer i, Integer j)
  {
    ARCCORE_CHECK_AT(i, 2);
    ARCCORE_CHECK_AT(j, 2);
    return (&x)[i][j];
  }

 public:

  //! Écrit le couple \a t dans le flux \a o
  friend std::ostream& operator<<(std::ostream& o, Real2x2 t)
  {
    return t.printXy(o);
  }

  //! Lit le couple \a t depuis le flux \a o.
  friend std::istream& operator>>(std::istream& i, Real2x2& t)
  {
    return t.assign(i);
  }

  //! Multiplication par un scalaire.
  friend constexpr ARCCORE_HOST_DEVICE Real2x2 operator*(Real sca, Real2x2 vec)
  {
    return Real2x2(vec.x * sca, vec.y * sca);
  }

  //! Multiplication par un scalaire.
  friend constexpr ARCCORE_HOST_DEVICE Real2x2 operator*(Real2x2 vec, Real sca)
  {
    return Real2x2(vec.x * sca, vec.y * sca);
  }

  //! Division par un scalaire.
  friend constexpr ARCCORE_HOST_DEVICE Real2x2 operator/(Real2x2 vec, Real sca)
  {
    return Real2x2(vec.x / sca, vec.y / sca);
  }

  /*!
  * \brief Opérateur de comparaison.
  *
  * Cet opérateur permet par exemple de trier Real2x2
  * dans std::set
  */
  friend constexpr ARCCORE_HOST_DEVICE bool operator<(Real2x2 v1, Real2x2 v2)
  {
    if (v1.x == v2.x) {
      return v1.y < v2.y;
    }
    return (v1.x < v2.x);
  }

 public:

  /*!
   * \brief Compare la matrice avec la matrice nulle.
   *
   * La matrice est nulle si et seulement si chacune de ses composantes
   * est inférieure à un epsilon donné. La valeur epsilon utilisée est celle
   * de float_info<value_type>::nearlyEpsilon():
   * \f[A=0 \Leftrightarrow |A.x|<\epsilon,|A.y|<\epsilon\f]
   *
   * \retval vrai si la matrice est égale à la matrice nulle,
   * \retval faux sinon.
   */
  // TODO: make obsolete mid-2025: ARCANE_DEPRECATED_REASON("Y2024: Use math::isNearlyZero(const Real2x2&) instead")
  inline constexpr ARCCORE_HOST_DEVICE bool isNearlyZero() const;

 private:

  /*!
   * \brief Compare les valeurs \a a et \a b avec le comparateur TypeEqualT
   * \retval vrai si \a a et \a b sont égaux,
   * \retval faux sinon.
   */
  constexpr ARCCORE_HOST_DEVICE static bool _eq(Real a, Real b)
  {
    return TypeEqualT<Real>::isEqual(a, b);
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
