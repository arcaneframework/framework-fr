// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* ArrayIterator.h                                             (C) 2000-2026 */
/*                                                                           */
/* Itérateur sur les Array, ArrayView, ConstArrayView, ...                   */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_ARRAYITERATOR_H
#define ARCCORE_BASE_ARRAYITERATOR_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/ArccoreGlobal.h"

#include <type_traits>
#include <cstddef>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane::Impl
{
//! Traits pour les itérateurs de tableaux. Ceux-ci doivent être spécialisés
template <typename T>
struct ArrayIteratorTraits;

//! Spécialisation pour pointeur.
template <typename T>
struct ArrayIteratorTraits<T*>
{
  using value_type = std::remove_cv_t<T>;
  using difference_type = std::ptrdiff_t;
  using reference = T&;
  using pointer = T*;
};

}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Itérateur sur les classes tableau de Arccore.
 *
 * Cet itérateur est utilisé pour les classes Array, ArrayView et ConstArrayView.
 *
 * Il est du type std::random_access_iterator_tag.
 */
template <typename Iterator_>
class ArrayIterator
{
 private:

  // Pour le cas où on ne supporte pas le C++14.
  template <bool B, class XX = void>
  using Iterator_enable_if_t = typename std::enable_if<B, XX>::type;

 protected:

  Iterator_ m_ptr;

  using TraitsType_ = Arcane::Impl::ArrayIteratorTraits<Iterator_>;

 public:

  //typedef typename std::random_access_iterator_tag iterator_category;
  //using iterator_category = std::random_access_iterator_tag;
  using value_type = TraitsType_::value_type;
  using difference_type = TraitsType_::difference_type;
  using reference = TraitsType_::reference;
  using pointer = TraitsType_::pointer;

 public:

  constexpr ARCCORE_HOST_DEVICE ArrayIterator() noexcept : m_ptr(Iterator_()) {}

  constexpr ARCCORE_HOST_DEVICE explicit ArrayIterator(const Iterator_& i) noexcept
  : m_ptr(i) {}

  // Permettre la conversion de l'itérateur en const_iterator
  template <typename X, typename = Iterator_enable_if_t<std::is_same<X, value_type*>::value>>
  constexpr ARCCORE_HOST_DEVICE ArrayIterator(const ArrayIterator<X>& iter) noexcept
  : m_ptr(iter.base()) {}

  // Exigences de l'itérateur avant
  constexpr ARCCORE_HOST_DEVICE reference operator*() const noexcept { return *m_ptr; }
  constexpr ARCCORE_HOST_DEVICE pointer operator->() const noexcept { return m_ptr; }
  constexpr ARCCORE_HOST_DEVICE ArrayIterator& operator++() noexcept
  {
    ++m_ptr;
    return *this;
  }
  constexpr ARCCORE_HOST_DEVICE ArrayIterator operator++(int) noexcept { return ArrayIterator(m_ptr++); }

  // Exigences de l'itérateur bidirectionnel
  constexpr ARCCORE_HOST_DEVICE ArrayIterator& operator--() noexcept
  {
    --m_ptr;
    return *this;
  }
  constexpr ARCCORE_HOST_DEVICE ArrayIterator operator--(int) noexcept { return ArrayIterator(m_ptr--); }

  // Exigences de l'itérateur d'accès aléatoire
  constexpr ARCCORE_HOST_DEVICE reference operator[](difference_type n) const noexcept { return m_ptr[n]; }
  constexpr ARCCORE_HOST_DEVICE ArrayIterator& operator+=(difference_type n) noexcept
  {
    m_ptr += n;
    return *this;
  }
  constexpr ARCCORE_HOST_DEVICE ArrayIterator operator+(difference_type n) const noexcept { return ArrayIterator(m_ptr + n); }
  constexpr ARCCORE_HOST_DEVICE ArrayIterator& operator-=(difference_type n) noexcept
  {
    m_ptr -= n;
    return *this;
  }
  constexpr ARCCORE_HOST_DEVICE ArrayIterator operator-(difference_type n) const noexcept { return ArrayIterator(m_ptr - n); }

  constexpr ARCCORE_HOST_DEVICE const Iterator_& base() const noexcept { return m_ptr; }
};

// Exigences de l'itérateur avant
template <typename I1, typename I2> constexpr ARCCORE_HOST_DEVICE inline bool
operator==(const ArrayIterator<I1>& lhs, const ArrayIterator<I2>& rhs) noexcept
{
  return lhs.base() == rhs.base();
}

template <typename I> constexpr ARCCORE_HOST_DEVICE inline bool
operator==(const ArrayIterator<I>& lhs, const ArrayIterator<I>& rhs) noexcept
{
  return lhs.base() == rhs.base();
}

template <typename I1, typename I2> constexpr ARCCORE_HOST_DEVICE inline bool
operator!=(const ArrayIterator<I1>& lhs, const ArrayIterator<I2>& rhs) noexcept
{
  return lhs.base() != rhs.base();
}

template <typename I> constexpr ARCCORE_HOST_DEVICE inline bool
operator!=(const ArrayIterator<I>& lhs, const ArrayIterator<I>& rhs) noexcept
{
  return lhs.base() != rhs.base();
}

// Exigences de l'itérateur d'accès aléatoire
template <typename I1, typename I2> constexpr ARCCORE_HOST_DEVICE inline bool
operator<(const ArrayIterator<I1>& lhs, const ArrayIterator<I2>& rhs) noexcept
{
  return lhs.base() < rhs.base();
}

template <typename I> constexpr ARCCORE_HOST_DEVICE inline bool
operator<(const ArrayIterator<I>& lhs, const ArrayIterator<I>& rhs) noexcept
{
  return lhs.base() < rhs.base();
}

template <typename I1, typename I2> constexpr ARCCORE_HOST_DEVICE inline bool
operator>(const ArrayIterator<I1>& lhs, const ArrayIterator<I2>& rhs) noexcept
{
  return lhs.base() > rhs.base();
}

template <typename I> constexpr ARCCORE_HOST_DEVICE inline bool
operator>(const ArrayIterator<I>& lhs, const ArrayIterator<I>& rhs) noexcept
{
  return lhs.base() > rhs.base();
}

template <typename I1, typename I2> constexpr ARCCORE_HOST_DEVICE inline bool
operator<=(const ArrayIterator<I1>& lhs, const ArrayIterator<I2>& rhs) noexcept
{
  return lhs.base() <= rhs.base();
}

template <typename I> constexpr ARCCORE_HOST_DEVICE inline bool
operator<=(const ArrayIterator<I>& lhs, const ArrayIterator<I>& rhs) noexcept
{
  return lhs.base() <= rhs.base();
}

template <typename I1, typename I2> constexpr ARCCORE_HOST_DEVICE inline bool
operator>=(const ArrayIterator<I1>& lhs, const ArrayIterator<I2>& rhs) noexcept
{
  return lhs.base() >= rhs.base();
}

template <typename I> constexpr ARCCORE_HOST_DEVICE inline bool
operator>=(const ArrayIterator<I>& lhs, const ArrayIterator<I>& rhs) noexcept
{
  return lhs.base() >= rhs.base();
}

// _GLIBCXX_RESOLVE_LIB_DEFECTS
// Selon la résolution de DR179, non seulement les divers opérateurs de comparaison
// mais aussi l'opérateur- doit accepter des paramètres d'itérateur/const_iterator mixtes.
template <typename I1, typename I2>
#if __cplusplus >= 201103L
// DR 685.
constexpr ARCCORE_HOST_DEVICE inline auto
operator-(const ArrayIterator<I1>& lhs, const ArrayIterator<I2>& rhs) noexcept
->decltype(lhs.base() - rhs.base())
#else
constexpr inline typename ArrayIterator<I1>::difference_type
operator-(const ArrayIterator<I1>& lhs, const ArrayIterator<I2>& rhs)
#endif
{
  return lhs.base() - rhs.base();
}

template <typename I> constexpr ARCCORE_HOST_DEVICE inline typename ArrayIterator<I>::difference_type
operator-(const ArrayIterator<I>& lhs, const ArrayIterator<I>& rhs) noexcept
{
  return lhs.base() - rhs.base();
}

template <typename I> constexpr ARCCORE_HOST_DEVICE inline ArrayIterator<I>
operator+(typename ArrayIterator<I>::difference_type n,
          const ArrayIterator<I>& i) noexcept
{
  return ArrayIterator<I>(i.base() + n);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
