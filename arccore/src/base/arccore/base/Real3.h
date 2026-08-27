// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* Real3.h                                                     (C) 2000-2026 */
/*                                                                           */
/* Vecteur de 3 dimensions de 'Real'.                                        */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_REAL3_H
#define ARCCORE_BASE_REAL3_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/ArccoreGlobal.h"
#include "arccore/base/Real2.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

struct Real3POD
{
 public:

  Real x; //!< première composante du triplet
  Real y; //!< deuxième composante du triplet
  Real z; //!< troisième composante du triplet

  /*!
   * Accès en lecture seule à la @a i-ème composante de Real3POD
   *
   * @note ne fonctionne que pour x, y et z dans l'ordre du POD
   *
   * @param i numéro de composante à retourner
   *
   * @return (&x)[i]
   */
  ARCCORE_HOST_DEVICE Real operator[](Integer i) const
  {
    ARCCORE_CHECK_AT(i, 3);
    return (&x)[i];
  }

  /*!
   * Accès en lecture seule à la @a i-ème composante de Real3POD
   *
   * @note ne fonctionne que pour x, y et z dans l'ordre du POD
   *
   * @param i numéro de composante à retourner
   *
   * @return (&x)[i]
   */
  ARCCORE_HOST_DEVICE Real operator()(Integer i) const
  {
    ARCCORE_CHECK_AT(i, 3);
    return (&x)[i];
  }

  /*!
   * Accès à la @a i-ème composante de Real3POD
   *
   * @note ne fonctionne que pour x, y et z dans l'ordre du POD
   *
   * @param i numéro de composante à retourner
   *
   * @return (&x)[i]
   */
  ARCCORE_HOST_DEVICE Real& operator[](Integer i)
  {
    ARCCORE_CHECK_AT(i, 3);
    return (&x)[i];
  }

  /*!
   * Accès à la @a i-ème composante de Real3POD
   *
   * @note ne fonctionne que pour x, y et z dans l'ordre du POD
   *
   * @param i numéro de composante à retourner
   *
   * @return (&x)[i]
   */
  ARCCORE_HOST_DEVICE Real& operator()(Integer i)
  {
    ARCCORE_CHECK_AT(i, 3);
    return (&x)[i];
  }

  //! Définit la \a i-ème composante à la \a valeur
  ARCCORE_HOST_DEVICE void setComponent(Integer i, Real value)
  {
    ARCCORE_CHECK_AT(i, 3);
    (&x)[i] = value;
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Classe gérant un vecteur réel de 3 dimensions.
 *
 * Le vector inclut trois composantes \a x, \a y et \a z, qui sont de type \b Real.
 *
 * \code
 * Real3 value (1.0,2.3,4.5); // Créé un triplet (x=1.0, y=2.3, z=4.5)
 * cout << value.x;           // Affiche la composante x 
 * value.y += 3.2;            // Ajoute 3.2 à la composante \b y
 * \endcode
 *
 * ou équivalemment
 *
 * \code
 * Real3 value (1.0,2.3,4.5); // Créé un triplet (x=1.0, y=2.3, z=4.5)
 * cout << value[0];          // Affiche la composante x 
 * value[1] += 3.2;           // Ajoute 3.2 à la composante \b y
 * \endcode
 */
class ARCCORE_BASE_EXPORT Real3
: public Real3POD
{
 public:

  //! Construit le vecteur nul.
  constexpr ARCCORE_HOST_DEVICE Real3()
  : Real3POD()
  {
    x = 0.0;
    y = 0.0;
    z = 0.0;
  }

  //! Construit le triplet (ax,ay,az)
  constexpr ARCCORE_HOST_DEVICE Real3(Real ax, Real ay, Real az)
  : Real3POD()
  {
    x = ax;
    y = ay;
    z = az;
  }

  //! Construit un triplet identique à \a f
  Real3(const Real3& f) = default;

  //! Construit un triplet identique à \a f
  constexpr ARCCORE_HOST_DEVICE explicit Real3(const Real3POD& f)
  : Real3POD()
  {
    x = f.x;
    y = f.y;
    z = f.z;
  }

  //! Construit l'instance avec le triplet (v,v,v).
  constexpr ARCCORE_HOST_DEVICE explicit Real3(Real v)
  : Real3POD()
  {
    x = y = z = v;
  }

  //! Construit un triplet identique à \a f
  constexpr ARCCORE_HOST_DEVICE explicit Real3(const Real2& f)
  : Real3POD()
  {
    x = f.x;
    y = f.y;
    z = 0.0;
  }

  //! Construit le triplet (av[0], av[1], av[2])
  constexpr ARCCORE_HOST_DEVICE Real3(ConstArrayView<Real> av)
  : Real3POD()
  {
    x = av[0];
    y = av[1];
    z = av[2];
  }

  //! Opérateur d'affectation de copie.
  Real3& operator=(const Real3& f) = default;

  //! Affecte le triplet (v,v,v) à l'instance.
  constexpr ARCCORE_HOST_DEVICE Real3& operator=(Real v)
  {
    x = y = z = v;
    return (*this);
  }

 public:

  constexpr ARCCORE_HOST_DEVICE static Real3 null() { return Real3(0., 0., 0.); }
  constexpr ARCCORE_HOST_DEVICE static Real3 zero() { return Real3(0., 0., 0.); }

 public:

  //! Retourne une copie du triplet.
  constexpr ARCCORE_HOST_DEVICE Real3 copy() const { return (*this); }

  //! Réinitialise le triplet en utilisant les constructeurs par défaut.
  constexpr ARCCORE_HOST_DEVICE Real3& reset()
  {
    x = y = z = 0.;
    return (*this);
  }

  //! Affecte le triplet (ax,ay,az) à l'instance.
  constexpr ARCCORE_HOST_DEVICE Real3& assign(Real ax, Real ay, Real az)
  {
    x = ax;
    y = ay;
    z = az;
    return (*this);
  }

  //! Copie le triplet \a f
  constexpr ARCCORE_HOST_DEVICE Real3& assign(Real3 f)
  {
    x = f.x;
    y = f.y;
    z = f.z;
    return (*this);
  }

  //! Retourne une vue des trois éléments du vecteur.
  constexpr ARCCORE_HOST_DEVICE ArrayView<Real> view()
  {
    return { 3, &x };
  }

  //! Retourne une vue constante des trois éléments du vecteur.
  constexpr ARCCORE_HOST_DEVICE ConstArrayView<Real> constView() const
  {
    return { 3, &x };
  }

  //! Valeur absolue composante par composante.
  ARCCORE_HOST_DEVICE Real3 absolute() const { return Real3(std::abs(x), std::abs(y), std::abs(z)); }

  /*!
   * \brief Lit un triplet depuis le flux \a i
   * Le triplet est lu sous la forme de trois valeurs de type #value_type.
   */
  std::istream& assign(std::istream& i);

  //! Écrit le triplet dans le flux \a o lisible par assign()
  std::ostream& print(std::ostream& o) const;

  //! Écrit le triplet dans le flux \a o sous la forme (x,y,z)
  std::ostream& printXyz(std::ostream& o) const;

  //! Ajoute \a b au triplet
  constexpr ARCCORE_HOST_DEVICE Real3& add(Real3 b)
  {
    x += b.x;
    y += b.y;
    z += b.z;
    return (*this);
  }

  //! Soustrait \a b du triplet
  constexpr ARCCORE_HOST_DEVICE Real3& sub(Real3 b)
  {
    x -= b.x;
    y -= b.y;
    z -= b.z;
    return (*this);
  }

  //! Multiplie chaque composante du triplet par la composante correspondante de \a b
  constexpr ARCCORE_HOST_DEVICE Real3& mul(Real3 b)
  {
    x *= b.x;
    y *= b.y;
    z *= b.z;
    return (*this);
  }

  //! Divise chaque composante du triplet par la composante correspondante de \a b
  constexpr ARCCORE_HOST_DEVICE Real3& div(Real3 b)
  {
    x /= b.x;
    y /= b.y;
    z /= b.z;
    return (*this);
  }

  //! Ajoute \a b à chaque composante du triplet
  constexpr ARCCORE_HOST_DEVICE Real3& addSame(Real b)
  {
    x += b;
    y += b;
    z += b;
    return (*this);
  }

  //! Soustrait b à chaque composante du triplet
  constexpr ARCCORE_HOST_DEVICE Real3& subSame(Real b)
  {
    x -= b;
    y -= b;
    z -= b;
    return (*this);
  }

  //! Multiplie chaque composante du triplet par b
  constexpr ARCCORE_HOST_DEVICE Real3& mulSame(Real b)
  {
    x *= b;
    y *= b;
    z *= b;
    return (*this);
  }

  //! Divise chaque composante du triplet par b
  constexpr ARCCORE_HOST_DEVICE Real3& divSame(Real b)
  {
    x /= b;
    y /= b;
    z /= b;
    return (*this);
  }

  //! Ajoute b au triplet.
  constexpr ARCCORE_HOST_DEVICE Real3& operator+=(Real3 b) { return add(b); }

  //! Soustrait b du triplet
  constexpr ARCCORE_HOST_DEVICE Real3& operator-=(Real3 b) { return sub(b); }

  //! Multiplie chaque composante du triplet par la composante correspondante de b
  constexpr ARCCORE_HOST_DEVICE Real3& operator*=(Real3 b) { return mul(b); }

  //! Multiplie chaque composante du triplet par le nombre réel b
  constexpr ARCCORE_HOST_DEVICE void operator*=(Real b)
  {
    x *= b;
    y *= b;
    z *= b;
  }

  //! Divise chaque composante du triplet par la composante correspondante de b
  constexpr ARCCORE_HOST_DEVICE Real3& operator/=(Real3 b) { return div(b); }

  //! Divise chaque composante du triplet par le nombre réel b
  constexpr ARCCORE_HOST_DEVICE void operator/=(Real b)
  {
    x /= b;
    y /= b;
    z /= b;
  }

  //! Crée un triplet qui est égal à ce triplet ajouté à b
  constexpr ARCCORE_HOST_DEVICE Real3 operator+(Real3 b) const { return Real3(x + b.x, y + b.y, z + b.z); }

  //! Crée un triplet qui est égal à b soustrait de ce triplet
  constexpr ARCCORE_HOST_DEVICE Real3 operator-(Real3 b) const { return Real3(x - b.x, y - b.y, z - b.z); }

  //! Crée un triplet opposé au triplet actuel
  constexpr ARCCORE_HOST_DEVICE Real3 operator-() const { return Real3(-x, -y, -z); }

  /*!
   * \brief Crée un triplet qui est égal à ce triplet dont chaque composante a été
   * multipliée par la composante correspondante de b.
   */
  constexpr ARCCORE_HOST_DEVICE Real3 operator*(Real3 b) const { return Real3(x * b.x, y * b.y, z * b.z); }

  /*!
   * \brief Crée un triplet qui est égal à ce triplet dont chaque composante a été divisée
   * par la composante correspondante de b.
   */
  constexpr ARCCORE_HOST_DEVICE Real3 operator/(Real3 b) const { return Real3(x / b.x, y / b.y, z / b.z); }

  //! Multiplication par un scalaire.
  friend constexpr ARCCORE_HOST_DEVICE Real3 operator*(Real sca, Real3 vec)
  {
    return Real3(vec.x * sca, vec.y * sca, vec.z * sca);
  }

  //! Multiplication par un scalaire.
  friend constexpr ARCCORE_HOST_DEVICE Real3 operator*(Real3 vec, Real sca)
  {
    return Real3(vec.x * sca, vec.y * sca, vec.z * sca);
  }

  //! Division par un scalaire.
  friend constexpr ARCCORE_HOST_DEVICE Real3 operator/(Real3 vec, Real sca)
  {
    return Real3(vec.x / sca, vec.y / sca, vec.z / sca);
  }

 public:

  /*!
   * \brief Opérateur de comparaison.
   *
   * Cet opérateur permet de trier Real3 par exemple
   * dans std::set
   */
  friend constexpr ARCCORE_HOST_DEVICE bool operator<(Real3 v1, Real3 v2)
  {
    if (v1.x == v2.x) {
      if (v1.y == v2.y)
        return v1.z < v2.z;
      else
        return v1.y < v2.y;
    }
    return (v1.x < v2.x);
  }

  //! Écrit le triplet t dans le flux o
  friend std::ostream& operator<<(std::ostream& o, Real3 t)
  {
    return t.printXyz(o);
  }

  //! Lit le triplet t depuis le flux o.
  friend std::istream& operator>>(std::istream& i, Real3& t)
  {
    return t.assign(i);
  }

  /*!
   * \brief Compare l'instance actuelle composante par composante à b.
   *
   * \retval vrai si this.x==b.x et this.y==b.y et this.z==b.z.
   * \retval faux sinon.
   */
  constexpr ARCCORE_HOST_DEVICE bool operator==(Real3 b) const
  {
    return _eq(x, b.x) && _eq(y, b.y) && _eq(z, b.z);
  }

  /*!
   * \brief Compare deux triplets.
   * Pour la notion d'égalité, voir operator==()
   * \retval vrai si les deux triplets sont différents,
   * \retval faux sinon.
   */
  constexpr ARCCORE_HOST_DEVICE bool operator!=(Real3 b) const { return !operator==(b); }

 public:

  //! Retourne le carré de la norme L2 du triplet $\f$x^2+y^2+z^2$\f$
  // TODO: deprecate mid-2025: ARCANE_DEPRECATED_REASON("Y2024: Use math::squareNormL2(const Real3&) instead")
  constexpr ARCCORE_HOST_DEVICE Real squareNormL2() const { return x * x + y * y + z * z; }

  //! Retourne la norme L2 du triplet $\f$\sqrt{x^2+y^2+z^2}\f$
  // TODO: deprecate mid-2025: ARCANE_DEPRECATED_REASON("Y2024: Use math::normL2(const Real3&) instead")
  inline ARCCORE_HOST_DEVICE Real normL2() const;

  //! Retourne le carré de la norme du triplet $\f$x^2+y^2+z^2\f$
  ARCCORE_DEPRECATED_2021("Use math::squareNormL2(const Real3&) instead")
  constexpr ARCCORE_HOST_DEVICE Real abs2() const { return x * x + y * y + z * z; }

  //! Retourne la norme du triplet $\f$\sqrt{x^2+y^2+z^2}\f$
  ARCCORE_DEPRECATED_2021("Use math::normL2(const Real3&) instead")
  inline ARCCORE_HOST_DEVICE Real abs() const;

  // TODO: deprecate mid-2025: ARCANE_DEPRECATED_REASON("Y2024: Use math::isNearlyZero(const Real3&) instead")
  inline constexpr ARCCORE_HOST_DEVICE bool isNearlyZero() const;

  // TODO: deprecate mid-2026: ARCANE_DEPRECATED_REASON("Y2024: Use math::mutableNormalize(Real3&) instead")
  inline Real3& normalize();

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

inline constexpr ARCCORE_HOST_DEVICE Real2::
Real2(const Real3& v)
: Real2POD()
{
  x = v.x;
  y = v.y;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
