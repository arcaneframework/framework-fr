// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* MathNumeric.h                                               (C) 2000-2026 */
/*                                                                           */
/* Opérations mathématiques sur les types numériques (Real2, Real3,          */
/* NumVector, ...).                                                          */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_MATHNUMERIC_H
#define ARCCORE_BASE_MATHNUMERIC_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/NumVector.h"
#include "arccore/base/NumMatrix.h"

#include "arccore/base/MathReal2.h"
#include "arccore/base/MathReal3.h"
#include "arccore/base/MathReal2x2.h"
#include "arccore/base/MathReal3x3.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane::math
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Compare le vecteur avec le vecteur nul.
 *
 * La matrice est presque nulle si et seulement si chacun de ses composants
 * est inférieur à un epsilon donné. La valeur epsilon utilisée est celle
 * de FloatInfo<DataType>::nearlyEpsilon():
 * \f[A=0 \Leftrightarrow |A.x|<\epsilon,|A.y|<\epsilon,|A.z|<\epsilon \f]
 */
template <typename DataType, int Size> constexpr ARCCORE_HOST_DEVICE bool
isNearlyZero(const NumVector<DataType, Size>& v)
{
  bool is_nearly_zero = true;
  for (int i = 0; i < Size; ++i)
    is_nearly_zero = is_nearly_zero && math::isNearlyZero(v[i]);
  return is_nearly_zero;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Compare la matrice avec la matrice nulle.
 *
 * La matrice est nulle si et seulement si chacun de ses composants
 * est inférieur à un epsilon donné. La valeur epsilon utilisée est celle
 * de float_info<value_type>::nearlyEpsilon():
 * \f[A=0 \Leftrightarrow |A.x|<\epsilon,|A.y|<\epsilon,|A.z|<\epsilon \f]
 *
 * \retval vrai si la matrice est égale à la matrice nulle,
 * \retval faux sinon.
 */
template <typename DataType, int RowSize, int ColumnSize> constexpr ARCCORE_HOST_DEVICE bool
isNearlyZero(const NumMatrix<DataType, RowSize, ColumnSize>& v)
{
  bool is_nearly_zero = true;
  for (int i = 0; i < RowSize; ++i)
    is_nearly_zero = is_nearly_zero && math::isNearlyZero(v.row(i));
  return is_nearly_zero;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

//! Retourne le carré de la norme L2 du vecteur
template <typename DataType, int Size> constexpr ARCCORE_HOST_DEVICE DataType
squareNormL2(const NumVector<DataType, Size>& v)
{
  DataType norm = {};
  for (int i = 0; i < Size; ++i)
    norm += v[i] * v[i];
  return norm;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

//! Retourne la norme L2 du vecteur
template <typename DataType, int Size> ARCCORE_HOST_DEVICE Real
normL2(const NumVector<DataType, Size>& v)
{
  return Arcane::math::sqrt(squareNormL2(v));
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane::math

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
