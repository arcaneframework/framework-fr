// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* Convert.h                                                   (C) 2000-2026 */
/*                                                                           */
/* Fonctions pour convertir une chaîne de caractère en un type donné.        */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_CONVERT_H
#define ARCCORE_BASE_CONVERT_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/StringView.h"

#ifndef ARCCORE_COMPILING_FRAMEWORK
// Ce fichier d'en-tête n'est pas nécessaire et sera supprimé en juillet 2027
#include <iostream>
#endif
#include <optional>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane::Convert
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

//! Convertit un \c Real en \c double
inline double
toDouble(Real r)
{
#ifdef ARCCORE_REAL_USE_APFLOAT
  return ap2double(r.ap);
#else
  return static_cast<double>(r);
#endif
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

//! Convertit un \c Real en \c Integer
inline Integer
toInteger(Real r)
{
  return static_cast<Integer>(toDouble(r));
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

//! Convertit un \c Real en \c Int64
inline Int64
toInt64(Real r)
{
  return static_cast<Int64>(toDouble(r));
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

//! Convertit un \c Real en \c Int32
inline Int32
toInt32(Real r)
{
  return static_cast<Int32>(toDouble(r));
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

//! Convertit un \c Real en \c Integer
inline bool
toBool(Real r)
{
  return static_cast<bool>(toDouble(r));
}

//! Convertit \c r en \c Real
inline Real
toReal(Real r)
{
  return r;
}

//! Convertit \c r en \c Real
inline Real
toReal(int r)
{
  return static_cast<Real>(r);
}

//! Convertit \c r en \c Real
inline Real
toReal(unsigned int r)
{
  return static_cast<Real>(r);
}

//! Convertit \c r en \c Real
inline Real
toReal(long r)
{
  return static_cast<Real>(r);
}

//! Convertit \c r en \c Real
inline Real
toReal(unsigned long r)
{
  return static_cast<Real>(r);
}

//! Convertit \c r en \c Real
inline Real
toReal(long long r)
{
#ifdef ARCCORE_REAL_USE_APFLOAT
  return static_cast<Real>(static_cast<long>(r));
#else
  return static_cast<Real>(r);
#endif
}

//! Convertit \c r en \c Real
inline Real
toReal(unsigned long long r)
{
#ifdef ARCCORE_REAL_USE_APFLOAT
  return static_cast<Real>(static_cast<unsigned long>(r));
#else
  return static_cast<Real>(r);
#endif
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Classe template pour convertir un type.
 *
 * Actuellement cela est uniquement disponible via une spécialisation
 * pour les types 'Int32', 'Int64' et 'Real3'.
 */
template <typename T>
class Type;

template <typename T>
class ScalarType
{
 public:

  //! Convertit \a s au type \a T
  ARCCORE_BASE_EXPORT static std::optional<T> tryParse(StringView s);

  /*!
   * \brief Convertit \a s au type \a T.
   *
   * Si \a s.empty() est vrai, alors retourne \a default_value.
   */
  static std::optional<T>
  tryParseIfNotEmpty(StringView s, const T& default_value)
  {
    return (s.empty()) ? default_value : tryParse(s);
  }

  /*!
   * \brief Convertit la valeur de la variable d'environnement \a s en le type \a T.
   *
   * Si platform::getEnvironmentVariable(s) est nul, return std::nullopt.
   * Sinon, retourne cette valeur convertie en le type \a T. Si la conversion
   * n'est pas possible, retourne std::nullopt si \a throw_if_invalid vaut \a false ou
   * lève une exception s'il vaut \a true.
   */
  ARCCORE_BASE_EXPORT static std::optional<T>
  tryParseFromEnvironment(StringView s, bool throw_if_invalid);
};

//! Spécialisation pour les types scalaires
template <> class Type<Int64> : public ScalarType<Int64>
{};
template <> class Type<Int32> : public ScalarType<Int32>
{};
template <> class Type<Real> : public ScalarType<Real>
{};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane::Convert

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
