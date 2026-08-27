// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* MathReal2.h                                                 (C) 2000-2026 */
/*                                                                           */
/* Opérations mathématiques sur Real2.                                       */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_MATHREAL2_H
#define ARCCORE_BASE_MATHREAL2_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/Real2.h"
#include "arccore/base/MathBase.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane::math
{
  /*!
   * \brief Indique si l'instance est proche de l'instance zéro.
   *
   * \retval true si math::isNearlyZero() est vrai pour chaque composante.
   * \retval faux sinon.
   */
  inline constexpr ARCCORE_HOST_DEVICE bool isNearlyZero(const Real2& v)
  {
    return math::isNearlyZero(v.x) && math::isNearlyZero(v.y);
  }

  //! Retourne la norme au carré du couple $\f$x^2+y^2+z^2$\f$
  inline constexpr ARCCORE_HOST_DEVICE Real squareNormL2(const Real2& v)
  {
    return v.x * v.x + v.y * v.y;
  }

  //! Retourne la norme du couple $\f$\sqrt{x^2+y^2+z^2}$\f$
  inline ARCCORE_HOST_DEVICE Real normL2(const Real2& v)
  {
    return math::sqrt(math::squareNormL2(v));
  }

  /*!
   * \brief Normalise le couple.
   *
   * Si le couple n'est pas nul, divise chaque composante par la norme du couple
   * (abs()), de sorte qu'après avoir appelé cette méthode, abs() soit égal à 1.
   * Si le couple est nul, ne fait rien.
   */
  inline Real2& mutableNormalize(Real2& v)
  {
    Real d = math::normL2(v);
    if (!math::isZero(d))
      v.divSame(d);
    return v;
  }

  /*!
    * \brief Retourne le couple v normalisé par la norme L2.
    *
    * Si `math::normL2(v)` n'est pas nul, retourne le couple v divisé par `math::normL2(v)`.
    * Sinon, retourne v.
    */
  inline Real2 normalizeL2(const Real2& v)
  {
    Real d = math::normL2(v);
    if (!math::isZero(d))
      return v / d;
    return v;
  }
} // namespace math

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

inline constexpr ARCCORE_HOST_DEVICE bool Real2::
isNearlyZero() const
{
  return math::isNearlyZero(*this);
}

inline ARCCORE_HOST_DEVICE Real Real2::
normL2() const
{
  return math::normL2(*this);
}

inline Real2& Real2::
normalize()
{
  return math::mutableNormalize(*this);
}

inline ARCCORE_HOST_DEVICE Real Real2::
abs() const
{
  return math::normL2(*this);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
