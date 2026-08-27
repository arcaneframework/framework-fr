// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* ArrayRange.h                                                (C) 2000-2026 */
/*                                                                           */
/* Intervalle sur les Array, ArrayView, ConstArrayView, ...                  */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_ARRAYRANGE_H
#define ARCCORE_BASE_ARRAYRANGE_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/ArrayIterator.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Intervalle sur les classes tableau de %Arccore.
 *
 * Cette classe est utilisée pour adapter les classes tableaux aux
 * itérateurs de la STL. Elle fourniy les méthodes telles que begin()/end().
 */
template <typename T>
class ArrayRange
{
 protected:

  using TraitsType_ = Arcane::Impl::ArrayIteratorTraits<T>;

 public:

  using value_type = TraitsType_::value_type;
  using difference_type = TraitsType_::difference_type;
  using reference = TraitsType_::reference;
  using pointer = TraitsType_::pointer;

  using const_pointer = const value_type*;
  //! Type de l'itérateur pour un élément du tableau
  using iterator = ArrayIterator<pointer>;
  //! Type de l'itérateur constant pour un élément du tableau
  using const_iterator = ArrayIterator<const_pointer>;

 public:

  //! Construit une plage vide.
  ArrayRange() = default;

  //! Construit une plage allant de \a abegin à \a aend.
  ArrayRange(pointer abegin, pointer aend) noexcept
  : m_begin(abegin)
  , m_end(aend)
  {}

 public:

  //! Retourne un itérateur sur le premier élément du tableau
  iterator begin() { return iterator(m_begin); }
  //! Retourne un iterateur sur le premier élément après la fin du tableau
  iterator end() { return iterator(m_end); }
  //! Retourne un iterateur constant sur le premier élément du tableau
  const_iterator begin() const { return const_iterator(m_begin); }
  //! Retourne un iterateur constant sur le premier élément après la fin du tableau
  const_iterator end() const { return const_iterator(m_end); }

  //! Pointeur sur le tableau sous-jacent.
  value_type* data() { return m_begin; }
  //! Pointeur constant sur le tableau sous-jacent.
  const value_type* data() const { return m_begin; }
  //! Indique si le tableau est vide.
  bool empty() const { return m_end == m_begin; }

 private:

  T m_begin = nullptr;
  T m_end  = nullptr;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
