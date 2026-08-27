// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* Real2.h                                                     (C) 2000-2026 */
/*                                                                           */
/* Vecteur 2-dimensionnel de 'Real'.                                         */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_REAL2_H
#define ARCCORE_BASE_REAL2_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/ArccoreGlobal.h"
#include "arccore/base/ArrayView.h"

#include <iosfwd>
#include <cstdlib>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

struct Real2POD
{
 public:

  Real x; //!< première composante du couple
  Real y; //!< deuxième composante du couple

  /*!
   * Accès en lecture seule à la @a i-ème composante de Real2POD
   *
   * @note ne fonctionne que si x, y sont ordonnés dans le POD
   *
   * @param i numéro de composante à retourner
   *
   * @return (&x)[i]
   */
  ARCCORE_HOST_DEVICE Real operator[](Integer i) const
  {
    ARCCORE_CHECK_AT(i, 2);
    return (&x)[i];
  }

  /*!
   * Accès en lecture seule à la @a i-ème composante de Real2POD
   *
   * @note ne fonctionne que si x, y sont ordonnés dans le POD
   *
   * @param i numéro de composante à retourner
   *
   * @return (&x)[i]
   */
  ARCCORE_HOST_DEVICE Real operator()(Integer i) const
  {
    ARCCORE_CHECK_AT(i, 2);
    return (&x)[i];
  }

  /*!
   * Accès à la @a i-ème composante de Real2POD
   *
   * @note ne fonctionne que si x, y sont ordonnés dans le POD
   *
   * @param i numéro de composante à retourner
   *
   * @return (&x)[i]
   */
  ARCCORE_HOST_DEVICE Real& operator[](Integer i)
  {
    ARCCORE_CHECK_AT(i, 2);
    return (&x)[i];
  }

  /*!
   * Accès à la @a i-ème composante de Real2POD
   *
   * @note ne fonctionne que si x, y sont ordonnés dans le POD
   *
   * @param i numéro de composante à retourner
   *
   * @return (&x)[i]
   */
  ARCCORE_HOST_DEVICE Real& operator()(Integer i)
  {
    ARCCORE_CHECK_AT(i, 2);
    return (&x)[i];
  }

  //! Positionne la \a i-ème composante à la valeur \a
  ARCCORE_HOST_DEVICE void setComponent(Integer i, Real value)
  {
    ARCCORE_CHECK_AT(i, 2);
    (&x)[i] = value;
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Classe gérant un vecteur réel 2-dimensionnel.
 *
 * Le vecteur comprend deux composantes \a x et \a y de type \b Real.
 *
 * \code
 * Real2 value (1.0,2.3); // Crée un couple (x=1.0, y=2.3)
 * cout << value.x;   // Affiche la composante x
 * value.y += 3.2; // Ajoute 3.2 à la composante \b y
 * \endcode
 */
class ARCCORE_BASE_EXPORT Real2
: public Real2POD
{
 public:

  //! Construit le vecteur nul.
  constexpr ARCCORE_HOST_DEVICE Real2()
  : Real2POD()
  {
    x = 0.;
    y = 0.;
  }
  //! Construit le couple (ax,ay)
  constexpr ARCCORE_HOST_DEVICE Real2(Real ax, Real ay)
  : Real2POD()
  {
    x = ax;
    y = ay;
  }
  //! Construit une copie identique à \a f
  Real2(const Real2& f) = default;
  //! Construit une copie identique à \a f
  constexpr ARCCORE_HOST_DEVICE explicit Real2(const Real2POD& f)
  : Real2POD()
  {
    x = f.x;
    y = f.y;
  }

  //! Construit l'instance avec le triplet (v,v,v).
  constexpr ARCCORE_HOST_DEVICE explicit Real2(Real v)
  : Real2POD()
  {
    x = y = v;
  }

  //! Construit l'instance en utilisant les deux premières composantes de Real3.
  inline constexpr ARCCORE_HOST_DEVICE explicit Real2(const Real3& v);

  //! Construit le couple (av[0], av[1])
  constexpr ARCCORE_HOST_DEVICE Real2(ConstArrayView<Real> av)
  : Real2POD()
  {
    x = av[0];
    y = av[1];
  }

  Real2& operator=(const Real2& f) = default;

  //! Assigne le couple (v,v) à l'instance.
  constexpr ARCCORE_HOST_DEVICE Real2& operator=(Real v)
  {
    x = y = v;
    return (*this);
  }

 public:

  constexpr ARCCORE_HOST_DEVICE static Real2 null() { return Real2(0., 0.); }

 public:

  //! Retourne une copie du couple.
  constexpr ARCCORE_HOST_DEVICE Real2 copy() const { return (*this); }

  //! Réinitialise le couple en utilisant les constructeurs par défaut.
  constexpr ARCCORE_HOST_DEVICE Real2& reset()
  {
    x = y = 0.0;
    return (*this);
  }

  //! Assigne le triplet (ax,ay,az) à l'instance
  constexpr ARCCORE_HOST_DEVICE Real2& assign(Real ax, Real ay)
  {
    x = ax;
    y = ay;
    return (*this);
  }

  //! Copie le couple \a f
  constexpr ARCCORE_HOST_DEVICE Real2& assign(Real2 f)
  {
    x = f.x;
    y = f.y;
    return (*this);
  }

  //! Retourne une vue des deux éléments du vecteur.
  constexpr ARCCORE_HOST_DEVICE ArrayView<Real> view()
  {
    return { 2, &x };
  }

  //! Retourne une vue constante des deux éléments du vecteur.
  constexpr ARCCORE_HOST_DEVICE ConstArrayView<Real> constView() const
  {
    return { 2, &x };
  }

  //! Valeur absolue composante par composante.
  inline ARCCORE_HOST_DEVICE Real2 absolute() const
  {
    return Real2(std::abs(x), std::abs(y));
  }

  /*!
   * \brief Lit un couple à partir du flux \a i
   * Le couple est lu sous la forme de trois valeurs de type #value_type.
   */
  std::istream& assign(std::istream& i);

  //! Écrit le couple dans le flux \a o lisible par assign()
  std::ostream& print(std::ostream& o) const;

  //! Écrit le couple dans le flux \a o sous la forme (x,y)
  std::ostream& printXy(std::ostream& o) const;

  //! Ajoute \a b au couple
  constexpr ARCCORE_HOST_DEVICE Real2& add(Real2 b)
  {
    x += b.x;
    y += b.y;
    return (*this);
  }

  //! Soustrait \a b du couple
  constexpr ARCCORE_HOST_DEVICE Real2& sub(Real2 b)
  {
    x -= b.x;
    y -= b.y;
    return (*this);
  }

  //! Multiplie chaque composante du couple par la composante correspondante de \a b
  constexpr ARCCORE_HOST_DEVICE Real2& mul(Real2 b)
  {
    x *= b.x;
    y *= b.y;
    return (*this);
  }

  //! Divise chaque composante du couple par la composante correspondante de \a b
  constexpr ARCCORE_HOST_DEVICE Real2& div(Real2 b)
  {
    x /= b.x;
    y /= b.y;
    return (*this);
  }

  //! Ajoute \a b à chaque composante du couple
  constexpr ARCCORE_HOST_DEVICE Real2& addSame(Real b)
  {
    x += b;
    y += b;
    return (*this);
  }

  //! Soustrait \a b à chaque composante du couple
  constexpr ARCCORE_HOST_DEVICE Real2& subSame(Real b)
  {
    x -= b;
    y -= b;
    return (*this);
  }

  //! Multiplie chaque composante du couple par b
  constexpr ARCCORE_HOST_DEVICE Real2& mulSame(Real b)
  {
    x *= b;
    y *= b;
    return (*this);
  }

  //! Divise chaque composante du couple par b
  constexpr ARCCORE_HOST_DEVICE Real2& divSame(Real b)
  {
    x /= b;
    y /= b;
    return (*this);
  }

  //! Ajoute b au couple.
  constexpr ARCCORE_HOST_DEVICE Real2& operator+=(Real2 b) { return add(b); }

  //! Soustrait b du couple
  constexpr ARCCORE_HOST_DEVICE Real2& operator-=(Real2 b) { return sub(b); }

  //! Multiplie chaque composante du couple par la composante correspondante de b
  constexpr ARCCORE_HOST_DEVICE Real2& operator*=(Real2 b) { return mul(b); }

  //! Multiplie chaque composante du couple par le nombre réel b
  constexpr ARCCORE_HOST_DEVICE void operator*=(Real b)
  {
    x *= b;
    y *= b;
  }

  //! Divise chaque composante du couple par la composante correspondante de b
  constexpr ARCCORE_HOST_DEVICE Real2& operator/=(Real2 b) { return div(b); }

  //! Divise chaque composante du couple par le nombre réel b
  constexpr ARCCORE_HOST_DEVICE void operator/=(Real b)
  {
    x /= b;
    y /= b;
  }

  //! Crée un couple égal à ce couple ajouté à b
  constexpr ARCCORE_HOST_DEVICE Real2 operator+(Real2 b) const { return Real2(x + b.x, y + b.y); }

  //! Crée un couple égal à b soustrait de ce couple
  constexpr ARCCORE_HOST_DEVICE Real2 operator-(Real2 b) const { return Real2(x - b.x, y - b.y); }

  //! Crée un couple opposé au couple actuel
  constexpr ARCCORE_HOST_DEVICE Real2 operator-() const { return Real2(-x, -y); }

  /*!
   * \brief Crée un couple égal à ce couple, où chaque composante a été
   * multipliée par la composante correspondante de b.
   */
  constexpr ARCCORE_HOST_DEVICE Real2 operator*(Real2 b) const { return Real2(x * b.x, y * b.y); }

  /*!
   * \brief Crée un couple égal à ce couple, où chaque composante a été divisée
   * par la composante correspondante de b.
   */
  constexpr ARCCORE_HOST_DEVICE Real2 operator/(Real2 b) const { return Real2(x / b.x, y / b.y); }

  //! Multiplication par un scalaire.
  friend constexpr ARCCORE_HOST_DEVICE Real2 operator*(Real sca, Real2 vec)
  {
    return Real2(vec.x * sca, vec.y * sca);
  }

  //! Multiplication par un scalaire.
  friend constexpr ARCCORE_HOST_DEVICE Real2 operator*(Real2 vec, Real sca)
  {
    return Real2(vec.x * sca, vec.y * sca);
  }

  //! Division par un scalaire.
  friend constexpr ARCCORE_HOST_DEVICE Real2 operator/(Real2 vec, Real sca)
  {
    return Real2(vec.x / sca, vec.y / sca);
  }

  /*!
   * \brief Opérateur de comparaison.
   *
   * Cet opérateur permet de trier les objets Real2 pour leur utilisation, par exemple,
   * dans std::set
   */
  friend constexpr ARCCORE_HOST_DEVICE bool operator<(Real2 v1, Real2 v2)
  {
    if (v1.x == v2.x) {
      return v1.y < v2.y;
    }
    return (v1.x < v2.x);
  }

  //! Écrit le couple t dans le flux o.
  friend std::ostream& operator<<(std::ostream& o, Real2 t)
  {
    return t.printXy(o);
  }

  //! Lit le couple t depuis le flux o.
  friend std::istream& operator>>(std::istream& i, Real2& t)
  {
    return t.assign(i);
  }

  /*!
   * \brief Compare l'instance actuelle composante par composante à b.
   *
   * \retval vrai si this.x==b.x et this.y==b.y.
   * \retval faux sinon.
   */
  constexpr ARCCORE_HOST_DEVICE bool operator==(Real2 b) const
  {
    return _eq(x, b.x) && _eq(y, b.y);
  }

  /*!
   * \brief Compare deux couples.
   * Pour le concept d'égalité, voir operator==()
   * \retval vrai si les deux couples sont différents,
   * \retval faux sinon.
   */
  constexpr ARCCORE_HOST_DEVICE bool operator!=(Real2 b) const { return !operator==(b); }

 public:

  //! Retourne la norme au carré du couple $\f$x^2+y^2+z^2$\f$
  // TODO: rendre obsolète à la mi-2025: ARCANE_DEPRECATED_REASON("Y2024: Utiliser math::squareNormL2(*this) à la place")
  constexpr ARCCORE_HOST_DEVICE Real squareNormL2() const { return x * x + y * y; }

  //! Retourne la norme au carré du couple $\f$x^2+y^2+z^2$\f$
  ARCCORE_DEPRECATED_2021("Use math::squareNormL2(*this) instead")
  ARCCORE_HOST_DEVICE Real abs2() const { return x * x + y * y; }

  //! Retourne la norme du couple $\f$\sqrt{x^2+y^2+z^2}$\f$
  ARCCORE_DEPRECATED_2021("Use math::normL2(*this) instead")
  inline ARCCORE_HOST_DEVICE Real abs() const;

  /*!
   * \brief Indique si l'instance est proche de l'instance nulle.
   *
   * \retval vrai si math::isNearlyZero() est vrai pour chaque composante.
   * \retval faux sinon.
   */
  // TODO: rendre obsolète à la mi-2025: ARCANE_DEPRECATED_REASON("Y2024: Utiliser math::isNearlyZero(const Real2&) à la place")
  inline constexpr ARCCORE_HOST_DEVICE bool isNearlyZero() const;

  //! Retourne la norme du couple $\f$\sqrt{x^2+y^2+z^2}$\f$
  // TODO: rendre obsolète à la mi-2025: ARCANE_DEPRECATED_REASON("Y2024: Utiliser math::normL2(const Real2&) à la place")
  ARCCORE_HOST_DEVICE Real normL2() const;

  // TODO: rendre obsolète à la mi-2026 ARCANE_DEPRECATED_REASON("Y2026: Utiliser math::mutableNormalize(Real2&) à la place")
  /*!
   * \brief Normalise le couple.
   *
   * Si le couple n'est pas nul, divise chaque composante par la norme du couple
   * (abs()), de sorte qu'après avoir appelé cette méthode, abs() soit égal à 1.
   * Si le couple est nul, ne fait rien.
   */
  inline Real2& normalize();

 private:

  /*!
   * \brief Compare les valeurs de a et b en utilisant le comparateur TypeEqualT
   * \retval vrai si a et b sont égaux,
   * \retval faux sinon.
   */
  constexpr ARCCORE_HOST_DEVICE static bool _eq(Real a, Real b)
  {
    return a == b;
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
