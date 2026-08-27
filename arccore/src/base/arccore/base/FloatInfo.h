// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* FloatInfo.h                                                 (C) 2000-2026 */
/*                                                                           */
/* Informations sur les limites des types à virgule flottante.               */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_FLOATINFO_H
#define ARCCORE_BASE_FLOATINFO_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/ArccoreGlobal.h"

#include <cfloat>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Informations sur le type à virgule flottante.
 * \note Spécialisation obligatoire pour les types à virgule flottante.
 */
template <typename T>
class FloatInfo
{
 public:

  //! Indique que l'instanciation est pour un type à virgule flottante.
  static constexpr bool isFloatType() { return false; }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Spécialisation de la classe FloatInfo pour le type \c float.
 */
template <>
class FloatInfo<float>
{
 public:

  //! Indique que l'instanciation est pour un type à virgule flottante.
  static constexpr bool isFloatType() { return true; }

 public:

  static constexpr unsigned int precision() { return 1; }
  static constexpr unsigned int maxDigit() { return FLT_DIG; }
  static constexpr float epsilon() { return FLT_EPSILON; }
  static constexpr float nearlyEpsilon() { return FLT_EPSILON * 10.0f; }
  static constexpr float maxValue() { return FLT_MAX; }
  static constexpr float zero() { return 0.0f; }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Spécialisation de la classe FloatInfo pour le type <tt>double</tt>.
 */
template <>
class FloatInfo<double>
{
 public:

  //! Indique que l'instanciation est pour un type à virgule flottante.
  static constexpr bool isFloatType() { return true; }

 public:

  static constexpr unsigned int precision() { return 2; }
  static constexpr unsigned int maxDigit() { return DBL_DIG; }
  static constexpr double epsilon() { return DBL_EPSILON; }
  static constexpr double nearlyEpsilon() { return DBL_EPSILON * 10.0; }
  static constexpr double maxValue() { return DBL_MAX; }
  static constexpr double zero() { return 0.0; }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Spécialisation de la classe FloatInfo pour le type
 * <tt>long double</tt>.
 */
template <>
class FloatInfo<long double>
{
 public:

  //! Indique que l'instanciation est pour un type à virgule flottante.
  static constexpr bool isFloatType() { return true; }

 public:

  static constexpr unsigned int precision() { return 3; }
  static constexpr unsigned int maxDigit() { return LDBL_DIG; }
  static constexpr long double epsilon() { return LDBL_EPSILON; }
  static constexpr long double nearlyEpsilon() { return LDBL_EPSILON * 10.0; }
  static constexpr long double maxValue() { return LDBL_MAX; }
  static constexpr long double zero() { return 0.0l; }
};

#ifdef ARCCORE_REAL_USE_APFLOAT
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*!
 * \brief Spécialisation de la classe FloatInfo pour le type
 * <tt>long double</tt>.
 *
 * \todo Vérifier que cette classe est valide pour toutes les architectures.
 */
template <>
class FloatInfo<apfloat>
{
 public:

  //! Indique que l'instanciation est pour un type à virgule flottante.
  //typedef TrueType _IsFloatType;
  //! Indique que l'instanciation est pour un type à virgule flottante.
  static constexpr bool isFloatType() { return true; }

 public:

  static constexpr unsigned int precision() { return 3; }
  static constexpr unsigned int maxDigit() { return 35; }
  static constexpr apfloat epsilon() { return 1e-30; }
  static constexpr apfloat nearlyEpsilon() { return 1e-28; }
  static constexpr apfloat maxValue() { return apfloat("1e1000"); }
  static constexpr apfloat zero() { return apfloat("0.0"); }
};
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
