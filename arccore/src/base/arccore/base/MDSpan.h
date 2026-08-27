// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* MDSpan.h                                                    (C) 2000-2026 */
/*                                                                           */
/* Vue sur un tableaux multi-dimensionnel pour les types numériques.         */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_MDSPAN_H
#define ARCCORE_BASE_MDSPAN_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/ArrayExtents.h"
#include "arccore/base/ArrayBounds.h"
#include "arccore/base/NumericTraits.h"
#include "arccore/base/ArrayLayout.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Classe de base des vues multi-dimensionnelles.
 *
 * Cette classe s'inspire la classe std::mdspan en cours de définition
 * (voir http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p0009r12.html)
 *
 * Cette classe est utilisée pour gérer les vues sur les tableaux tels que
 * NumArray. Les méthodes de cette classe sont accessibles sur accélérateur.
 *
 * Pour plus d'informations, se reporter à la page \ref arcanedoc_core_types_numarray.
 */
template <typename DataType_, typename Extents_, typename LayoutPolicy_>
class MDSpan
{
  using UnqualifiedValueType = std::remove_cv_t<DataType_>;
  friend class NumArray<UnqualifiedValueType, Extents_, LayoutPolicy_>;
  // Pour que MDSpan<const T> ait accès à MDSpan<T>
  friend class MDSpan<const UnqualifiedValueType, Extents_, LayoutPolicy_>;
  using ThatClass = MDSpan<DataType_, Extents_, LayoutPolicy_>;
  static constexpr bool IsConst = std::is_const_v<DataType_>;

 public:

  using DataType = DataType_;
  using Extents = Extents_;
  using LayoutPolicy = LayoutPolicy_;
  using value_type = DataType;
  using ExtentsType = Extents;
  using ExtentIndexType = Extents::ExtentIndexType;
  using LayoutPolicyType = LayoutPolicy;
  using MDIndexType = typename Extents::MDIndexType;
  using LoopIndexType = MDIndexType;
  using ArrayExtentsWithOffsetType = ArrayExtentsWithOffset<Extents, LayoutPolicy>;
  using DynamicDimsType = typename Extents::DynamicDimsType;
  using RemovedFirstExtentsType = typename Extents::RemovedFirstExtentsType;
  using ConstMDSpanType = MDSpan<const DataType, Extents, LayoutPolicy>;
  // Pour compatibilité. A supprimer pour cohérence avec les autres 'using'
  using ArrayBoundsIndexType = typename Extents::MDIndexType;
  using IndexType = typename Extents::MDIndexType;

 public:

  MDSpan() = default;
  ~MDSpan() = default;
  constexpr MDSpan(DataType* ptr, ArrayExtentsWithOffsetType extents) noexcept
  : m_ptr(ptr)
  , m_extents(extents)
  {
  }
  constexpr MDSpan(DataType* ptr, const DynamicDimsType& dims) noexcept
  : m_ptr(ptr)
  , m_extents(dims)
  {}
  constexpr MDSpan(const MDSpan<UnqualifiedValueType, Extents, LayoutPolicy>& rhs) noexcept requires(IsConst)
  : m_ptr(rhs.m_ptr)
  , m_extents(rhs.m_extents)
  {}
  MDSpan(const MDSpan<DataType, Extents, LayoutPolicy>& rhs) = default;
  constexpr MDSpan(SmallSpan<DataType> v) noexcept
  requires(Extents::isDynamic1D())
  : m_ptr(v.data())
  , m_extents(DynamicDimsType(v.size()))
  {}
  constexpr MDSpan(ArrayView<UnqualifiedValueType> v) noexcept
  requires(Extents::isDynamic1D())
  : m_ptr(v.data())
  , m_extents(DynamicDimsType(v.size()))
  {}
  constexpr MDSpan(SmallSpan<UnqualifiedValueType> v) noexcept
  requires(Extents::isDynamic1D() && IsConst)
  : m_ptr(v.data())
  , m_extents(DynamicDimsType(v.size()))
  {}
  constexpr MDSpan(ConstArrayView<UnqualifiedValueType> v) noexcept
  requires(Extents::isDynamic1D() && IsConst)
  : m_ptr(v.data())
  , m_extents(DynamicDimsType(v.size()))
  {}
  constexpr ThatClass& operator=(SmallSpan<DataType> v) noexcept
  requires(Extents::isDynamic1D())
  {
    m_ptr = v.data();
    m_extents = DynamicDimsType(v.size());
    return (*this);
  }
  constexpr ThatClass& operator=(SmallSpan<UnqualifiedValueType> v) noexcept
  requires(Extents::isDynamic1D() && IsConst)
  {
    m_ptr = v.data();
    m_extents = DynamicDimsType(v.size());
    return (*this);
  }
  constexpr ThatClass& operator=(ArrayView<UnqualifiedValueType> v) noexcept
  requires(Extents::isDynamic1D())
  {
    m_ptr = v.data();
    m_extents = DynamicDimsType(v.size());
    return (*this);
  }
  constexpr ThatClass& operator=(ConstArrayView<UnqualifiedValueType> v) noexcept
  requires(Extents::isDynamic1D() && IsConst)
  {
    m_ptr = v.data();
    m_extents = DynamicDimsType(v.size());
    return (*this);
  }
  MDSpan& operator=(const MDSpan& v) = default;
  MDSpan& operator=(MDSpan&& v) noexcept = default;

 public:

  //! Pointeur de base pour la mémoire allouée
  constexpr DataType* data() noexcept { return m_ptr; }
  //! Pointeur de base pour la mémoire allouée
  constexpr const DataType* data() const noexcept { return m_ptr; }

 public:

  constexpr DataType* _internalData() { return m_ptr; }
  constexpr const DataType* _internalData() const { return m_ptr; }

 public:

  ArrayExtents<Extents> extents() const
  {
    return m_extents.extents();
  }
  ArrayExtentsWithOffsetType extentsWithOffset() const
  {
    return m_extents;
  }

 public:

  //! Valeur de la première dimension
  constexpr ExtentIndexType extent0() const requires(Extents::rank() >= 1) { return m_extents.extent0(); }
  //! Valeur de la deuxième dimension
  constexpr ExtentIndexType extent1() const requires(Extents::rank() >= 2) { return m_extents.extent1(); }
  //! Valeur de la troisième dimension
  constexpr ExtentIndexType extent2() const requires(Extents::rank() >= 3) { return m_extents.extent2(); }
  //! Valeur de la quatrième dimension
  constexpr ExtentIndexType extent3() const requires(Extents::rank() >= 4) { return m_extents.extent3(); }

 public:

  //! Valeur pour l'élément \a i,j,k,l
  constexpr Int64 offset(ExtentIndexType i, ExtentIndexType j,
                         ExtentIndexType k, ExtentIndexType l) const
  requires(Extents::rank() == 4)
  {
    return m_extents.offset(i, j, k, l);
  }
  //! Valeur pour l'élément \a i,j,k
  constexpr Int64 offset(ExtentIndexType i, ExtentIndexType j,
                         ExtentIndexType k) const
  requires(Extents::rank() == 3)
  {
    return m_extents.offset(i, j, k);
  }
  //! Valeur pour l'élément \a i,j
  constexpr Int64 offset(ExtentIndexType i, ExtentIndexType j) const
  requires(Extents::rank() == 2)
  {
    return m_extents.offset(i, j);
  }
  //! Valeur pour l'élément \a i
  constexpr Int64 offset(ExtentIndexType i) const
  requires(Extents::rank() == 1) { return m_extents.offset(i); }

  //! Valeur pour l'élément \a idx
  constexpr Int64 offset(MDIndexType idx) const
  {
    return m_extents.offset(idx);
  }

 public:

  //! Valeur pour l'élément \a i,j,k,l
  constexpr DataType& operator()(ExtentIndexType i, ExtentIndexType j,
                                 ExtentIndexType k, ExtentIndexType l) const
  requires(Extents::rank() == 4)
  {
    return m_ptr[offset(i, j, k, l)];
  }
  //! Valeur pour l'élément \a i,j,k
  constexpr DataType& operator()(ExtentIndexType i, ExtentIndexType j, ExtentIndexType k) const
  requires(Extents::rank() == 3)
  {
    return m_ptr[offset(i, j, k)];
  }
  //! Valeur pour l'élément \a i,j
  constexpr DataType& operator()(ExtentIndexType i, ExtentIndexType j) const
  requires(Extents::rank() == 2)
  {
    return m_ptr[offset(i, j)];
  }
  //! Valeur pour l'élément \a i
  constexpr DataType& operator()(ExtentIndexType i) const
  requires(Extents::rank() == 1)
  {
    return m_ptr[offset(i)];
  }
  //! Valeur pour l'élément \a i
  constexpr DataType operator[](ExtentIndexType i) const
  requires(Extents::rank() == 1)
  {
    return m_ptr[offset(i)];
  }

  //! Valeur pour l'élément \a idx
  constexpr DataType& operator()(MDIndexType idx) const
  {
    return m_ptr[offset(idx)];
  }

 public:

  //! Pointeur sur la valeur pour l'élément \a i,j,k
  constexpr DataType* ptrAt(ExtentIndexType i, ExtentIndexType j,
                            ExtentIndexType k, ExtentIndexType l) const
  requires(Extents::rank() == 4)
  {
    return m_ptr + offset(i, j, k, l);
  }
  //! Pointeur sur la valeur pour l'élément \a i,j,k
  constexpr DataType* ptrAt(ExtentIndexType i, ExtentIndexType j,
                            ExtentIndexType k) const
  requires(Extents::rank() == 3)
  {
    return m_ptr + offset(i, j, k);
  }
  //! Pointeur sur la valeur pour l'élément \a i,j
  constexpr DataType* ptrAt(ExtentIndexType i, ExtentIndexType j) const
  requires(Extents::rank() == 2)
  {
    return m_ptr + offset(i, j);
  }
  //! Pointeur sur la valeur pour l'élément \a i
  constexpr DataType* ptrAt(ExtentIndexType i) const
  requires(Extents::rank() == 1)
  {
    return m_ptr + offset(i);
  }

  //! Pointeur sur la valeur pour l'élément \a i
  constexpr DataType* ptrAt(MDIndexType idx) const
  {
    return m_ptr + offset(idx);
  }

 public:

  /*!
   * \brief Retourne une vue de dimension (N-1) à partir de l'élément d'indice \a i.
   *
   * Par exemple :
   * \code
   *   MDSpan<Real, MDDim3> span3 = ...;
   *   MDSpan<Real, MDDim2> sliced_span = span3.slice(5);
   *   // sliced_span(i,i) <=> span3(5,i,j);
   * \endcode
   *
   * \warning Cela n'est valide que si \a LayoutPolicy est \a RightLayout.
   */
  ARCCORE_HOST_DEVICE MDSpan<DataType, RemovedFirstExtentsType, LayoutPolicy>
  slice(ExtentIndexType i) const requires(Extents::rank() >= 2 && std::is_base_of_v<RightLayout, LayoutPolicy>)
  {
    auto new_extents = m_extents.extents().removeFirstExtent().dynamicExtents();
    std::array<ExtentIndexType, ExtentsType::rank()> indexes = {};
    indexes[0] = i;
    DataType* base_ptr = this->ptrAt(MDIndexType(indexes));
    return MDSpan<DataType, RemovedFirstExtentsType, LayoutPolicy>(base_ptr, new_extents);
  }

 public:

  constexpr MDSpan<const DataType, Extents, LayoutPolicy> constSpan() const
  {
    return MDSpan<const DataType, Extents, LayoutPolicy>(m_ptr, m_extents);
  }

  constexpr MDSpan<const DataType, Extents, LayoutPolicy> constMDSpan() const
  {
    return MDSpan<const DataType, Extents, LayoutPolicy>(m_ptr, m_extents);
  }

  constexpr Span<DataType> to1DSpan() const
  {
    return { m_ptr, m_extents.totalNbElement() };
  }

  constexpr SmallSpan<DataType> to1DSmallSpan() requires(Extents::rank() == 1)
  {
    // TODO: peut-être ajouter un test en mode Check pour s'assurer que extent0() tient dans un 'Int32'
    return { _internalData(), static_cast<Int32>(extent0()) };
  }
  constexpr SmallSpan<const DataType> to1DSmallSpan() const requires(Extents::rank() == 1)
  {
    return to1DConstSmallSpan();
  }
  constexpr SmallSpan<const DataType> to1DConstSmallSpan() const requires(Extents::rank() == 1)
  {
    // TODO: peut-être ajouter un test en mode Check pour s'assurer que extent0() tient dans un 'Int32'
    return { _internalData(), static_cast<Int32>(extent0()) };
  }

 private:

  DataType* m_ptr = nullptr;
  ArrayExtentsWithOffsetType m_extents;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
