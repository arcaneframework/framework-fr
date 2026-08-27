// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* Math.h                                                      (C) 2000-2026 */
/*                                                                           */
/* Fonctions mathématiques diverses.                                         */
/*---------------------------------------------------------------------------*/
#ifndef ARCANE_UTILS_MATH_H
#define ARCANE_UTILS_MATH_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/utils/UtilsTypes.h"
#ifndef ARCCORE_COMPILING_FRAMEWORK
#include "arcane/utils/Convert.h"
#endif
#include "arccore/base/MathBase.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane::math
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*!
 * \brief Tronque la précision du réel \a v à \a nb_digit chiffres significatifs.
 *
 * Pour un réel double précision en IEEE 754, le nombre de bits significatif
 * est de 15 ou 16 suivant la valeur. Il est à noter qu'il n'est possible
 * de manière simple et rapide de tronquer la précision à une valeur donnée.
 * C'est pourquoi \a nb_digit représente un nombre de chiffre approximatif.
 * Notamment, il n'est pas possible de descendre en dessous de 8 chiffres
 * significatifs.
 *
 * Si \a nb_digit est inférieur ou égal à zéro ou supérieur à 15, c'est
 * la valeur \a v qui est retourné.
 */
extern ARCANE_UTILS_EXPORT double
truncateDouble(double v, Integer nb_digit);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*!
 * \brief Tronque la précision du tableau de réels \a values à
 * \a nb_digit chiffres significatifs.
 *
 * En sortie, chaque élément de \a values est modifié comme après appel
 * à truncateDouble(double v,Integer nb_digit).
 */
extern ARCANE_UTILS_EXPORT void
truncateDouble(ArrayView<double> values, Integer nb_digit);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arcane::math

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#ifdef ARCANE_REAL_USE_APFLOAT
#include "arcane/utils/MathApfloat.h"
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
