// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* ArrayExtents.h                                              (C) 2000-2026 */
/*                                                                           */
/* Gestion du nombre d'éléments par dimension pour les tableaux N-dimensions.*/
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_ARRAYEXTENTS_H
#define ARCCORE_BASE_ARRAYEXTENTS_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/ArrayView.h"
#include "arccore/base/MDIndex.h"
#include "arccore/base/ArrayLayout.h"
#include "arccore/base/ArrayExtentsValue.h"

#include "arccore/base/Span.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Spécialisation de ArrayStrideBase pour les tableaux de dimension 0 (les scalaires)
 */
template <>
class ArrayStridesBase<0>
{
 public:

  ArrayStridesBase() = default;
  //! Valeur du pas de la \a i-ème dimension.
  ARCCORE_HOST_DEVICE SmallSpan<const Int32> asSpan() const { return {}; }
  //! Value totale du pas
  ARCCORE_HOST_DEVICE Int64 totalStride() const { return 1; }
  ARCCORE_HOST_DEVICE static ArrayStridesBase<0> fromSpan([[maybe_unused]] Span<const Int32> strides)
  {
    // TODO: vérifier la taille de \a strides
    return {};
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*!
 * \brief Classe pour conserver le pas dans chaque dimension.
 *
 * Le pas pour une dimension est la distance en mémoire entre deux éléments
 * du tableau pour cette dimension. En général le pas est égal au nombre
 * d'éléments dans la dimension sauf si on utilise des marges (padding) par
 * exemple pour aligner certaines dimensions.
 */
template <int RankValue>
class ArrayStridesBase
{
 public:

  ArrayStridesBase() = default;
  //! Valeur du pas de la \a i-ème dimension.
  ARCCORE_HOST_DEVICE Int32 stride(int i) const { return m_strides[i]; }
  ARCCORE_HOST_DEVICE Int32 operator()(int i) const { return m_strides[i]; }
  ARCCORE_HOST_DEVICE SmallSpan<const Int32> asSpan() const { return { m_strides.data(), RankValue }; }
  //! Valeur totale du pas
  ARCCORE_HOST_DEVICE Int64 totalStride() const
  {
    Int64 nb_element = 1;
    for (int i = 0; i < RankValue; i++)
      nb_element *= m_strides[i];
    return nb_element;
  }
  // Instance contenant les dimensions après la première
  ARCCORE_HOST_DEVICE ArrayStridesBase<RankValue - 1> removeFirstStride() const
  {
    return ArrayStridesBase<RankValue - 1>::fromSpan({ m_strides.data() + 1, RankValue - 1 });
  }
  /*!
   * \brief Construit une instance à partir des valeurs données dans \a stride.
   * \pre stride.size() == RankValue.
   */
  ARCCORE_HOST_DEVICE static ArrayStridesBase<RankValue> fromSpan(Span<const Int32> strides)
  {
    ArrayStridesBase<RankValue> v;
    // TODO: vérifier la taille
    for (int i = 0; i < RankValue; ++i)
      v.m_strides[i] = strides[i];
    return v;
  }

 protected:

  std::array<Int32, RankValue> m_strides = {};
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Spécialisation de ArrayExtentsBase pour les tableaux de dimension 0 (les scalaires)
 */
template <>
class ArrayExtentsBase<ExtentsV<>>
{
 public:

  ArrayExtentsBase() = default;
  //! Nombre d'élément de la \a i-ème dimension.
  constexpr ARCCORE_HOST_DEVICE SmallSpan<const Int32> asSpan() const { return {}; }
  //! Nombre total d'eléments
  constexpr ARCCORE_HOST_DEVICE Int32 totalNbElement() const { return 1; }
  ARCCORE_HOST_DEVICE static ArrayExtentsBase<ExtentsV<>> fromSpan([[maybe_unused]] Span<const Int32> extents)
  {
    // TODO: vérifier la taille de \a extents
    return {};
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Classe pour conserver le nombre d'éléments dans chaque dimension.
 */
template <typename Extents>
class ArrayExtentsBase
: protected Extents::ArrayExtentsValueType
{
 protected:

  using BaseClass = Extents::ArrayExtentsValueType;
  using ExtentIndexType = Extents::ExtentIndexType;
  using ArrayExtentsPreviousRank = ArrayExtentsBase<typename Extents::RemovedFirstExtentsType>;
  using DynamicDimsType = Extents::DynamicDimsType;

 public:

  using BaseClass::asOtherStdArray;
  using BaseClass::asStdArray;
  using BaseClass::constExtent;
  using BaseClass::dynamicExtents;
  using BaseClass::getIndices;
  using BaseClass::totalNbElement;

 public:

  ARCCORE_HOST_DEVICE constexpr ArrayExtentsBase()
  : BaseClass()
  {}

 protected:

  explicit constexpr ARCCORE_HOST_DEVICE ArrayExtentsBase(SmallSpan<const ExtentIndexType> extents)
  : BaseClass(extents)
  {
  }

  explicit constexpr ARCCORE_HOST_DEVICE ArrayExtentsBase(DynamicDimsType extents)
  : BaseClass(extents)
  {
  }

 public:

  //! TEMPORAIRE : Définit le nombre d'éléments de la dimension 0 à \a v.
  ARCCORE_HOST_DEVICE void setExtent0(ExtentIndexType v) { this->m_extent0.v = v; }

  // Instance contenant les dimensions après la première
  ARCCORE_HOST_DEVICE ArrayExtentsPreviousRank removeFirstExtent() const
  {
    auto x = BaseClass::_removeFirstExtent();
    return ArrayExtentsPreviousRank::fromSpan(x);
  }

  // Nombre d'éléments de la I-ème dimension converti en 'Int64'.
  template <Int32 I> constexpr Int64 constLargeExtent() const
  {
    return BaseClass::template constExtent<I>();
  }

  /*!
   * \brief Construit une instance à partir des valeurs données dans \a extents.
   */
  ARCCORE_HOST_DEVICE static ArrayExtentsBase fromSpan(SmallSpan<const ExtentIndexType> extents)
  {
    return ArrayExtentsBase(extents);
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Extent pour les tableaux à 1 dimension.
 */
template <typename IndexType_, Int32 X0>
class ArrayExtents<ExtentsV<IndexType_, X0>>
: public ArrayExtentsBase<ExtentsV<IndexType_, X0>>
{
 public:

  using ExtentsType = ExtentsV<IndexType_, X0>;
  using BaseClass = ArrayExtentsBase<ExtentsType>;
  using BaseClass::extent0;
  using BaseClass::totalNbElement;
  using DynamicDimsType = BaseClass::DynamicDimsType;
  using MDIndexType = BaseClass::MDIndexType;
  using ExtentIndexType = BaseClass::ExtentIndexType;

 public:

  ArrayExtents() = default;
  constexpr ArrayExtents(const BaseClass& rhs)
  : BaseClass(rhs)
  {}
  constexpr ArrayExtents(const DynamicDimsType& extents)
  : BaseClass(extents)
  {
  }
  // TODO: À supprimer
  constexpr explicit ArrayExtents(ExtentIndexType dim1_size)
  {
    static_assert(ExtentsType::nb_dynamic == 1, "Cette méthode n'est autorisée que pour les extents entièrement dynamiques");
    this->m_extent0.v = dim1_size;
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Extent pour les tableaux à 2 dimensions.
 */
template <typename IndexType_, Int32 X0, Int32 X1>
class ArrayExtents<ExtentsV<IndexType_, X0, X1>>
: public ArrayExtentsBase<ExtentsV<IndexType_, X0, X1>>
{
 public:

  using ExtentsType = ExtentsV<IndexType_, X0, X1>;
  using BaseClass = ArrayExtentsBase<ExtentsType>;
  using BaseClass::extent0;
  using BaseClass::extent1;
  using BaseClass::totalNbElement;
  using DynamicDimsType = BaseClass::DynamicDimsType;
  using MDIndexType = BaseClass::MDIndexType;
  using ExtentIndexType = BaseClass::ExtentIndexType;

 public:

  ArrayExtents() = default;
  constexpr ArrayExtents(const BaseClass& rhs)
  : BaseClass(rhs)
  {}
  constexpr ArrayExtents(const DynamicDimsType& extents)
  : BaseClass(extents)
  {
  }
  // TODO: À supprimer
  constexpr ArrayExtents(ExtentIndexType dim1_size, ExtentIndexType dim2_size)
  {
    static_assert(ExtentsType::nb_dynamic == 2, "Cette méthode n'est autorisée que pour les extents entièrement dynamiques");
    this->m_extent0.v = dim1_size;
    this->m_extent1.v = dim2_size;
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Extent pour les tableaux à 3 dimensions.
 */
template <typename IndexType_, Int32 X0, Int32 X1, Int32 X2>
class ArrayExtents<ExtentsV<IndexType_, X0, X1, X2>>
: public ArrayExtentsBase<ExtentsV<IndexType_, X0, X1, X2>>
{
 public:

  using ExtentsType = ExtentsV<IndexType_, X0, X1, X2>;
  using BaseClass = ArrayExtentsBase<ExtentsType>;
  using BaseClass::extent0;
  using BaseClass::extent1;
  using BaseClass::extent2;
  using BaseClass::totalNbElement;
  using DynamicDimsType = BaseClass::DynamicDimsType;
  using MDIndexType = BaseClass::MDIndexType;
  using ExtentIndexType = BaseClass::ExtentIndexType;

 public:

  ArrayExtents() = default;
  constexpr ArrayExtents(const BaseClass& rhs)
  : BaseClass(rhs)
  {}
  constexpr ArrayExtents(const DynamicDimsType& extents)
  : BaseClass(extents)
  {
  }
  // TODO: À supprimer
  constexpr ArrayExtents(ExtentIndexType dim1_size, ExtentIndexType dim2_size, ExtentIndexType dim3_size)
  {
    static_assert(ExtentsType::nb_dynamic == 3, "Cette méthode n'est autorisée que pour les extents entièrement dynamiques");
    this->m_extent0.v = dim1_size;
    this->m_extent1.v = dim2_size;
    this->m_extent2.v = dim3_size;
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Extent pour les tableaux à 4 dimensions.
 */
template <typename IndexType_, Int32 X0, Int32 X1, Int32 X2, Int32 X3>
class ArrayExtents<ExtentsV<IndexType_, X0, X1, X2, X3>>
: public ArrayExtentsBase<ExtentsV<IndexType_, X0, X1, X2, X3>>
{
 public:

  using ExtentsType = ExtentsV<IndexType_, X0, X1, X2, X3>;
  using BaseClass = ArrayExtentsBase<ExtentsType>;
  using BaseClass::extent0;
  using BaseClass::extent1;
  using BaseClass::extent2;
  using BaseClass::extent3;
  using BaseClass::totalNbElement;
  using DynamicDimsType = BaseClass::DynamicDimsType;
  using MDIndexType = BaseClass::MDIndexType;
  using ExtentIndexType = BaseClass::ExtentIndexType;

 public:

  ArrayExtents() = default;
  constexpr ARCCORE_HOST_DEVICE ArrayExtents(const BaseClass& rhs)
  : BaseClass(rhs)
  {}
  constexpr ARCCORE_HOST_DEVICE ArrayExtents(const DynamicDimsType& extents)
  : BaseClass(extents)
  {
  }
  // TODO: À supprimer
  constexpr ARCCORE_HOST_DEVICE ArrayExtents(ExtentIndexType dim1_size, ExtentIndexType dim2_size, ExtentIndexType dim3_size, ExtentIndexType dim4_size)
  {
    static_assert(ExtentsType::nb_dynamic == 4, "Cette méthode n'est autorisée que pour les extents entièrement dynamiques");
    this->m_extent0.v = dim1_size;
    this->m_extent1.v = dim2_size;
    this->m_extent2.v = dim3_size;
    this->m_extent3.v = dim4_size;
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Extent et Offset pour les tableaux à 1 dimension.
 */
template <typename IndexType_, Int32 X0, typename LayoutType>
class ArrayExtentsWithOffset<ExtentsV<IndexType_, X0>, LayoutType>
: private ArrayExtents<ExtentsV<IndexType_, X0>>
{
 public:

  using ExtentsType = ExtentsV<IndexType_, X0>;
  using BaseClass = ArrayExtents<ExtentsType>;
  using BaseClass::asStdArray;
  using BaseClass::extent0;
  using BaseClass::getIndices;
  using BaseClass::totalNbElement;
  using Layout = typename LayoutType::Layout1Type;
  using DynamicDimsType = BaseClass::DynamicDimsType;
  using MDIndexType = BaseClass::MDIndexType;
  using ExtentIndexType = BaseClass::ExtentIndexType;

  using IndexType ARCCORE_DEPRECATED_REASON("Utilisez 'MDIndexType' à la place") = MDIndexType;

 public:

  ArrayExtentsWithOffset() = default;
  // TODO: à supprimer
  constexpr ARCCORE_HOST_DEVICE ArrayExtentsWithOffset(const ArrayExtents<ExtentsType>& rhs)
  : BaseClass(rhs)
  {
  }
  constexpr ARCCORE_HOST_DEVICE ArrayExtentsWithOffset(const DynamicDimsType& rhs)
  : BaseClass(rhs)
  {
  }
  constexpr ARCCORE_HOST_DEVICE Int64 offset(ExtentIndexType i) const
  {
    BaseClass::_checkIndex(i);
    return i;
  }
  constexpr ARCCORE_HOST_DEVICE Int64 offset(MDIndexType idx) const
  {
    BaseClass::_checkIndex(idx.id0());
    return idx.id0();
  }
  constexpr BaseClass extents() const
  {
    const BaseClass* b = this;
    return *b;
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Extent et Offset pour les tableaux à 2 dimensions.
 */
template <typename IndexType_, Int32 X0, Int32 X1, typename LayoutType>
class ArrayExtentsWithOffset<ExtentsV<IndexType_, X0, X1>, LayoutType>
: private ArrayExtents<ExtentsV<IndexType_, X0, X1>>
{
 public:

  using ExtentsType = ExtentsV<IndexType_, X0, X1>;
  using BaseClass = ArrayExtents<ExtentsType>;
  using BaseClass::asStdArray;
  using BaseClass::extent0;
  using BaseClass::extent1;
  using BaseClass::getIndices;
  using BaseClass::totalNbElement;
  using Layout = typename LayoutType::Layout2Type;
  using DynamicDimsType = BaseClass::DynamicDimsType;
  using MDIndexType = BaseClass::MDIndexType;
  using ExtentIndexType = BaseClass::ExtentIndexType;

  using IndexType ARCCORE_DEPRECATED_REASON("Utilisez 'MDIndexType' à la place") = MDIndexType;

 public:

  ArrayExtentsWithOffset() = default;
  // TODO: à supprimer
  constexpr ArrayExtentsWithOffset(ArrayExtents<ExtentsType> rhs)
  : BaseClass(rhs)
  {
  }
  constexpr ArrayExtentsWithOffset(const DynamicDimsType& rhs)
  : BaseClass(rhs)
  {
  }
  constexpr Int64 offset(ExtentIndexType i, ExtentIndexType j) const
  {
    return offset({ i, j });
  }
  constexpr Int64 offset(MDIndexType idx) const
  {
    BaseClass::_checkIndex(idx);
    return Layout::offset(idx, this->template constExtent<Layout::LastExtent>());
  }
  constexpr BaseClass extents() const
  {
    const BaseClass* b = this;
    return *b;
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Extent et Offset pour les tableaux à 3 dimensions.
 */
template <typename IndexType_, Int32 X0, Int32 X1, Int32 X2, typename LayoutType>
class ArrayExtentsWithOffset<ExtentsV<IndexType_, X0, X1, X2>, LayoutType>
: private ArrayExtents<ExtentsV<IndexType_, X0, X1, X2>>
{
 public:

  using ExtentsType = ExtentsV<IndexType_, X0, X1, X2>;
  using BaseClass = ArrayExtents<ExtentsType>;
  using BaseClass::asStdArray;
  using BaseClass::extent0;
  using BaseClass::extent1;
  using BaseClass::extent2;
  using BaseClass::getIndices;
  using BaseClass::totalNbElement;
  using Layout = typename LayoutType::Layout3Type;
  using DynamicDimsType = BaseClass::DynamicDimsType;
  using MDIndexType = BaseClass::MDIndexType;
  using ExtentIndexType = BaseClass::ExtentIndexType;

  using IndexType ARCCORE_DEPRECATED_REASON("Utilisez 'MDIndexType' à la place") = typename BaseClass::MDIndexType;

 public:

  ArrayExtentsWithOffset() = default;
  // TODO: à supprimer
  constexpr ArrayExtentsWithOffset(ArrayExtents<ExtentsType> rhs)
  : BaseClass(rhs)
  {
    _computeOffsets();
  }
  constexpr ArrayExtentsWithOffset(const DynamicDimsType& rhs)
  : BaseClass(rhs)
  {
    _computeOffsets();
  }
  constexpr Int64 offset(ExtentIndexType i, ExtentIndexType j, ExtentIndexType k) const
  {
    return offset({ i, j, k });
  }
  constexpr Int64 offset(MDIndexType idx) const
  {
    this->_checkIndex(idx);
    return Layout::offset(idx, this->template constExtent<Layout::LastExtent>(), m_dim23_size);
  }
  constexpr BaseClass extents() const
  {
    const BaseClass* b = this;
    return *b;
  }

 protected:

  ARCCORE_HOST_DEVICE void _computeOffsets()
  {
    const BaseClass& b = *this;
    m_dim23_size = Layout::computeOffsetIndexes(b);
  }

 private:

  Int64 m_dim23_size = 0;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Extent et Offset pour les tableaux à 4 dimensions.
 */
template <typename IndexType_, Int32 X0, Int32 X1, Int32 X2, Int32 X3, typename LayoutType>
class ArrayExtentsWithOffset<ExtentsV<IndexType_, X0, X1, X2, X3>, LayoutType>
: private ArrayExtents<ExtentsV<IndexType_, X0, X1, X2, X3>>
{
 public:

  using ExtentsType = ExtentsV<IndexType_, X0, X1, X2, X3>;
  using BaseClass = ArrayExtents<ExtentsType>;
  using BaseClass::asStdArray;
  using BaseClass::extent0;
  using BaseClass::extent1;
  using BaseClass::extent2;
  using BaseClass::extent3;
  using BaseClass::getIndices;
  using BaseClass::totalNbElement;
  using Layout = LayoutType::Layout4Type;
  using DynamicDimsType = BaseClass::DynamicDimsType;
  using MDIndexType = BaseClass::MDIndexType;
  using ExtentIndexType = BaseClass::ExtentIndexType;

  using IndexType ARCCORE_DEPRECATED_REASON("Utilisez 'MDIndexType' à la place") = typename BaseClass::MDIndexType;

 public:

  ArrayExtentsWithOffset() = default;
  // TODO: à supprimer
  constexpr ArrayExtentsWithOffset(ArrayExtents<ExtentsType> rhs)
  : BaseClass(rhs)
  {
    _computeOffsets();
  }
  constexpr ArrayExtentsWithOffset(const DynamicDimsType& rhs)
  : BaseClass(rhs)
  {
    _computeOffsets();
  }
  constexpr Int64 offset(ExtentIndexType i, ExtentIndexType j, ExtentIndexType k, ExtentIndexType l) const
  {
    return offset({ i, j, k, l });
  }
  constexpr Int64 offset(MDIndexType idx) const
  {
    this->_checkIndex(idx);
    return (m_dim234_size * idx.largeId0()) + m_dim34_size * idx.largeId1() + this->m_extent3.v * idx.largeId2() + idx.largeId3();
  }
  BaseClass extents() const
  {
    const BaseClass* b = this;
    return *b;
  }

 protected:

  ARCCORE_HOST_DEVICE void _computeOffsets()
  {
    m_dim34_size = Int64(this->m_extent2.v) * Int64(this->m_extent3.v);
    m_dim234_size = Int64(m_dim34_size) * Int64(this->m_extent1.v);
  }

 private:

  Int64 m_dim34_size = 0; //!< dim3 * dim4
  Int64 m_dim234_size = 0; //!< dim2 * dim3 * dim4
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
