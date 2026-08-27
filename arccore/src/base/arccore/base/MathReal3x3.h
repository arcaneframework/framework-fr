// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* MathReal3x3.h                                               (C) 2000-2026 */
/*                                                                           */
/* Matrice 3x3 de 'Real'.                                                    */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_MATHREAL3X3_H
#define ARCCORE_BASE_MATHREAL3X3_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/Real3x3.h"
#include "arccore/base/MathReal3.h"

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
 * \f[A=0 \Leftrightarrow |A.x|<\epsilon,|A.y|<\epsilon,|A.z|<\epsilon \f]
 *
 * \retval true si la matrice est égale à la matrice nulle,
 * \retval false sinon.
 */
inline constexpr ARCCORE_HOST_DEVICE bool isNearlyZero(const Real3x3& v)
{
  return isNearlyZero(v.x) && isNearlyZero(v.y) && isNearlyZero(v.z);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane::math

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

inline constexpr ARCCORE_HOST_DEVICE bool Real3x3::
isNearlyZero() const
{
  return math::isNearlyZero(*this);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // Fin de l'espace de noms Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
