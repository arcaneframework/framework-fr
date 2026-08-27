// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* MathReal2x2.h                                               (C) 2000-2026 */
/*                                                                           */
/* Opérations mathématiques sur Real2x2.                                     */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_MATHREAL2X2_H
#define ARCCORE_BASE_MATHREAL2X2_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/Real2x2.h"
#include "arccore/base/MathReal2.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane::math
{
/*!
 * \brief Compare la matrice avec la matrice nulle.
 *
 * La matrice est nulle si et seulement si chacun de ses composants
 * est inférieur à un epsilon donné. La valeur epsilon utilisée est celle
 * de float_info<value_type>::nearlyEpsilon():
 * \f[A=0 \Leftrightarrow |A.x|<\epsilon,|A.y|<\epsilon\f]
 *
 * \retval true si la matrice est égale à la matrice nulle,
 * \retval false sinon.
 */
constexpr ARCCORE_HOST_DEVICE bool isNearlyZero(const Real2x2& v)
{
  return math::isNearlyZero(v.x) && math::isNearlyZero(v.y);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane::math

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

inline constexpr ARCCORE_HOST_DEVICE bool Real2x2::
isNearlyZero() const
{
  return math::isNearlyZero(*this);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
