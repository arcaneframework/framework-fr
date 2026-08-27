// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* MathReal3.h                                                 (C) 2000-2026 */
/*                                                                           */
/* Opérations mathématiques sur Real3.                                       */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_MATHREAL3_H
#define ARCCORE_BASE_MATHREAL3_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/Real3.h"
#include "arccore/base/MathBase.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace math
{
  //! Retourne le carré de la norme L2 du triplet $\f$x^2+y^2+z^2\f$
  inline constexpr ARCCORE_HOST_DEVICE Real squareNormL2(const Real3& v)
  {
    return v.x * v.x + v.y * v.y + v.z * v.z;
  }

  /*!
   * \brief Indique si l'instance est proche de l'instance nulle.
   *
   * \retval true si math::isNearlyZero() est vrai pour chaque composante.
   * \retval false sinon.
   */
  inline constexpr ARCCORE_HOST_DEVICE bool isNearlyZero(const Real3& v)
  {
    return math::isNearlyZero(v.x) && math::isNearlyZero(v.y) && math::isNearlyZero(v.z);
  }

  //! Retourne la norme L2 du triplet $\f$\sqrt{v.x^2+v.y^2+v.z^2}\f$
  inline ARCCORE_HOST_DEVICE Real normL2(const Real3& v)
  {
    return math::sqrt(math::squareNormL2(v));
  }

  /*!
    * \brief Normalise le triplet v
    *
    * Si le triplet n'est pas nul, divise chaque composante par la norme du triplet
    * (abs()), de sorte qu'après avoir appelé cette méthode, math::normL2() soit égal à 1.
    * Si le triplet est nul, ne fait rien.
    */
  inline Real3& mutableNormalize(Real3& v)
  {
    Real d = math::normL2(v);
    if (!math::isZero(d))
      v.divSame(d);
    return v;
  }

  /*!
    * \brief Retourne le triplet v normalisé avec la norme L2.
    *
    * Si \`math::normL2(v)\` n'est pas nul, retourne le triplet v divisé par \`math::normL2(v)\`.
    * Sinon, retourne v.
    */
  inline Real3 normalizeL2(const Real3& v)
  {
    Real d = math::normL2(v);
    if (!math::isZero(d))
      return v / d;
    return v;
  }
} // namespace math

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

inline Real3& Real3::
normalize()
{
  return math::mutableNormalize(*this);
}

inline constexpr ARCCORE_HOST_DEVICE bool Real3::
isNearlyZero() const
{
  return math::isNearlyZero(*this);
}

inline ARCCORE_HOST_DEVICE Real Real3::
normL2() const
{
  return math::normL2(*this);
}

inline ARCCORE_HOST_DEVICE Real Real3::
abs() const
{
  return math::normL2(*this);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
