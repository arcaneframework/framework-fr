// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* TypeEqual.h                                                 (C) 2000-2026 */
/*                                                                           */
/* Gère l'égalité avec prise en charge de l'epsilon pour les types flottants.*/
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_TYPEEQUAL_H
#define ARCCORE_BASE_TYPEEQUAL_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/FloatInfo.h"
#include "arccore/base/BaseTypes.h"

// Pour 'std::abs'.
#include <cstdlib>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \internal
 * \brief Opérations de comparaison pour un type numérique T
 *
 * Cette classe définit un opérateur de comparaison pour le
 * type de paramètre de modèle 'T'. Il existe deux types de comparaisons :
 * - comparaisons exactes (isEqual());
 * - comparaisons approximatives (isNearlyEqual()).
 *
 * Les deux types de comparaisons sont identiques, sauf pour
 * les types à virgule flottante ou leurs équivalents. Dans ce cas, la comparaison exacte
 * compare les deux valeurs bit par bit, et la comparaison approximative
 * considère deux nombres comme égaux si leur différence relative est
 * inférieure à un epsilon.
 */
template <class T>
class TypeEqualT
{
 public:

  /*!
   * \brief Compare \a a à zéro.
   * \retval true si \a a est zéro dans un epsilon,
   * \retval false sinon.
   */
  constexpr ARCCORE_HOST_DEVICE static bool isNearlyZero(const T& a)
  {
    return (a == T());
  }

  /*!
   * \brief Compare \a a à zéro.
   * \retval true si \a a est exactement zéro,
   * \retval false sinon.
   */
  constexpr ARCCORE_HOST_DEVICE static bool isZero(const T& a)
  {
    return (a == T());
  }

  /*!
   * \brief Compare \a a à \a b.
   * \retval true si \a a et \b sont égaux dans un epsilon,
   * \retval false sinon.
   */
  constexpr ARCCORE_HOST_DEVICE static bool isNearlyEqual(const T& a, const T& b)
  {
    return (a == b);
  }

  /*!
   * \brief Compare \a a à \a b.
   * \retval true si \a a et \b sont égaux dans un epsilon,
   * \retval false sinon.
   */
  constexpr ARCCORE_HOST_DEVICE static bool isNearlyEqualWithEpsilon(const T& a, const T& b, const T&)
  {
    return (a == b);
  }

  /*!
   * \brief Compare \a a à \a b.
   * \retval true si \a a et \b sont exactement égaux,
   * \retval false sinon.
   */
  constexpr ARCCORE_HOST_DEVICE static bool isEqual(const T& a, const T& b)
  {
    return (a == b);
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*!
 * \internal
 * \brief Définit l'opérateur == pour les flottants.
 *
 * \note Finalement, il devrait utiliser la classe 'numeric_limits'
 * de la STL lorsqu'elle sera implémentée.
 */
template <class T>
class FloatEqualT
{
 private:

  constexpr ARCCORE_HOST_DEVICE static T nepsilon() { return FloatInfo<T>::nearlyEpsilon(); }

 public:

  constexpr ARCCORE_HOST_DEVICE static bool isNearlyZero(T a)
  {
    return ((a < 0.) ? a > -nepsilon() : a < nepsilon());
  }

  /*!
   * \brief Compare \a a à zéro dans \a epsilon.
   *
   * \a epsilon doit être positif.
   *
   * \retval true si abs(a)<epilon
   * \retval false sinon
   */
  constexpr ARCCORE_HOST_DEVICE static bool isNearlyZeroWithEpsilon(T a, T epsilon)
  {
    return ((a < 0.) ? a > -epsilon : a < epsilon);
  }

  /*! \brief Compare \a avec \a b*epsilon.
   * \warning b doit être positif. */
  ARCCORE_HOST_DEVICE static bool isNearlyZero(T a, T b)
  {
    return ((a < 0.) ? a > -(b * nepsilon()) : a < (b * nepsilon()));
  }

  constexpr ARCCORE_HOST_DEVICE static bool isTrueZero(T a) { return (a == FloatInfo<T>::zero()); }
  constexpr ARCCORE_HOST_DEVICE static bool isZero(T a) { return (a == FloatInfo<T>::zero()); }
  constexpr ARCCORE_HOST_DEVICE static bool isNearlyEqual(T a, T b)
  {
    T s = std::abs(a) + std::abs(b);
    T d = a - b;
    return (d == FloatInfo<T>::zero()) ? true : isNearlyZero(d / s);
  }
  constexpr ARCCORE_HOST_DEVICE static bool isNearlyEqualWithEpsilon(T a, T b, T epsilon)
  {
    T s = std::abs(a) + std::abs(b);
    T d = a - b;
    return (d == FloatInfo<T>::zero()) ? true : isNearlyZeroWithEpsilon(d / s, epsilon);
  }
  constexpr ARCCORE_HOST_DEVICE static bool isEqual(T a, T b)
  {
    return a == b;
  }
};

/*!
 * \internal
 * \brief Spécialisation de TypeEqualT pour le type <tt>float</tt>.
 */
template <>
class TypeEqualT<float>
: public FloatEqualT<float>
{};

/*!
 * \internal
 * \brief Spécialisation de TypeEqualT pour le type <tt>double</tt>.
 */
template <>
class TypeEqualT<double>
: public FloatEqualT<double>
{};

/*!
 * \internal
 * \brief Spécialisation de TypeEqualT pour le type <tt>long double</tt>.
 */
template <>
class TypeEqualT<long double>
: public FloatEqualT<long double>
{};

#ifdef ARCCORE_REAL_NOT_BUILTIN
/*!
 * \internal
 * \brief Spécialisation de TypeEqualT pour le type <tt>Real</tt>.
 */
template <>
class TypeEqualT<Real>
: public FloatEqualT<Real>
{};
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane::math
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Teste si deux valeurs sont approximativement égales.
 * Pour les types entiers, cette fonction est équivalente à IsEqual().
 * Dans le cas des types réels, les deux nombres sont considérés comme égaux
 * si et seulement si la valeur absolue de leur différence relative est
 * inférieure à un epsilon donné. Cet
 * epsilon est égal à float_info<Type>::nearlyEpsilon().
 * \retval true si les deux valeurs sont égales,
 * \retval false sinon.
 */
template <class Type> constexpr ARCCORE_HOST_DEVICE inline bool
isNearlyEqual(const Type& a, const Type& b)
{
  return TypeEqualT<Type>::isNearlyEqual(a, b);
}

//! Overload pour les réels
constexpr ARCCORE_HOST_DEVICE inline bool
isNearlyEqual(Real a, Real b)
{
  return TypeEqualT<Real>::isNearlyEqual(a, b);
}

/*!
 * \brief Teste si deux valeurs sont approximativement égales.
 * Pour les types entiers, cette fonction est équivalente à IsEqual().
 * Dans le cas des types réels, les deux nombres sont considérés comme égaux
 * si et seulement si la valeur absolue de leur différence relative est
 * inférieure à \a epsilon.
 *
 * \retval true si les deux valeurs sont égales,
 * \retval false sinon.
 */
template <class Type> constexpr ARCCORE_HOST_DEVICE inline bool
isNearlyEqualWithEpsilon(const Type& a, const Type& b, const Type& epsilon)
{
  return TypeEqualT<Type>::isNearlyEqualWithEpsilon(a, b, epsilon);
}

//! Overload pour les réels
ARCCORE_HOST_DEVICE constexpr inline bool
isNearlyEqualWithEpsilon(Real a, Real b, Real epsilon)
{
  return TypeEqualT<Real>::isNearlyEqualWithEpsilon(a, b, epsilon);
}

/*!
 * \brief Teste l'égalité bit par bit entre deux valeurs.
 * \retval true si les deux valeurs sont égales,
 * \retval false sinon.
 */
template <class Type> constexpr ARCCORE_HOST_DEVICE inline bool
isEqual(const Type& a, const Type& b)
{
  return TypeEqualT<Type>::isEqual(a, b);
}

//! Overload pour les réels
ARCCORE_HOST_DEVICE constexpr inline bool
isEqual(Real a, Real b)
{
  return TypeEqualT<Real>::isEqual(a, b);
}

/*!
 * \brief Teste si une valeur est approximativement égale à zéro dans un epsilon.
 *
 * Pour les types entiers, cette fonction est équivalente à IsZero().
 * Dans le cas des types réels, la valeur est considérée comme égale à
 * zéro si et seulement si sa valeur absolue est inférieure à un epsilon
 * donné par la fonction float_info<Type>::nearlyEpsilon().
 * \retval true si les deux valeurs sont égales,
 * \retval false sinon.
 */
template <class Type> constexpr ARCCORE_HOST_DEVICE inline bool
isNearlyZeroWithEpsilon(const Type& a, const Type& epsilon)
{
  return TypeEqualT<Type>::isNearlyZeroWithEpsilon(a, epsilon);
}

/*!
 * \brief Teste si une valeur est approximativement égale à zéro en utilisant l'epsilon standard.
 *
 * L'epsilon standard est celui retourné par FloatInfo<Type>::nearlyEpsilon().
 *
 * \sa isNearlyZero(const Type& a,const Type& epsilon).
 */
template <class Type> constexpr ARCCORE_HOST_DEVICE inline bool
isNearlyZero(const Type& a)
{
  return TypeEqualT<Type>::isNearlyZero(a);
}

/*!
 * \brief Teste si une valeur est exactement égale à zéro.
 * \retval true si \a est zéro,
 * \retval false sinon.
 */
template <class Type> constexpr ARCCORE_HOST_DEVICE inline bool
isZero(const Type& a)
{
  return TypeEqualT<Type>::isZero(a);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// Ces méthodes sont définies dans MathNumeric.h.
// Nous plaçons leur déclaration ici pour nous assurer que nous n'utilisons pas le
// TypeEqual générique par modèle

constexpr ARCCORE_HOST_DEVICE bool isNearlyZero(const Real2& a);
constexpr ARCCORE_HOST_DEVICE bool isNearlyZero(const Real3& a);
constexpr ARCCORE_HOST_DEVICE bool isNearlyZero(const Real2x2& a);
constexpr ARCCORE_HOST_DEVICE bool isNearlyZero(const Real3x3& a);

template <typename DataType, int Size> constexpr ARCCORE_HOST_DEVICE bool
isNearlyZero(const NumVector<DataType, Size>& v);
template <typename DataType, int RowSize, int ColumnSize> constexpr ARCCORE_HOST_DEVICE bool
isNearlyZero(const NumMatrix<DataType, RowSize, ColumnSize>& v);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane::math

namespace Arcane::Math
{
using Arcane::math::isEqual;
using Arcane::math::isNearlyEqual;
using Arcane::math::isNearlyEqualWithEpsilon;
} // namespace Arcane::Math

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
