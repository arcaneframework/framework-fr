// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* MeshMDVariableRef.h                                         (C) 2000-2026 */
/*                                                                           */
/* Classe gérant une variable multi-dimension sur une entité du maillage.    */
/*---------------------------------------------------------------------------*/
#ifndef ARCANE_CORE_MESHMDVARIABLEREF_H
#define ARCANE_CORE_MESHMDVARIABLEREF_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/utils/ArrayLayout.h"
#include "arcane/utils/ArrayShape.h"
#include "arcane/utils/MDSpan.h"

#include "arcane/core/DataView.h"

#include "arcane/core/MeshVariableArrayRef.h"
#include "arcane/core/datatype/DataTypeTraits.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane::Impl
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

template <typename ItemType, typename DataType>
class MeshMDVariableRefWrapperT
: public MeshVariableArrayRefT<ItemType, DataType>
{
  template <typename _ItemType, typename _DataType, typename _Extents>
  friend class Arcane::MeshMDVariableRefBaseT;

 public:

  using BaseClass = MeshVariableArrayRefT<ItemType, DataType>;
  using VariableType = typename BaseClass::PrivatePartType;
  using ValueDataType = typename VariableType::ValueDataType;

 private:

  explicit MeshMDVariableRefWrapperT(const VariableBuildInfo& vbi)
  : BaseClass(vbi)
  {
  }

 private:

  ValueDataType* trueData() { return this->m_private_part->trueData(); }
  const ValueDataType* trueData() const { return this->m_private_part->trueData(); }

  void fillShape(ArrayShape& shape_with_item)
  {
    this->m_private_part->fillShape(shape_with_item);
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane::Impl

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Classe de base gérant une variable multi-dimension sur une entité du maillage.
 */
template <typename ItemType, typename DataType, typename Extents>
class MeshMDVariableRefBaseT
: public MeshVariableRef
{
 public:

  using UnderlyingVariableType = MeshVariableArrayRefT<ItemType, DataType>;
  using MDSpanType = MDSpan<DataType, Extents, RightLayout>;
  using ItemLocalIdType = typename ItemType::LocalIdType;
  using FullExtentsType = Extents;

 public:

  explicit MeshMDVariableRefBaseT(const VariableBuildInfo& b)
  : MeshVariableRef(b)
  , m_underlying_var(b)
  {
    _internalInit(m_underlying_var.variable());
  }

  //! Variable sous-jacente associée.
  const UnderlyingVariableType& underlyingVariable() const { return m_underlying_var; }

  //! Variable sous-jacente associée.
  UnderlyingVariableType& underlyingVariable() { return m_underlying_var; }

  //! Forme complète (statique + dynamique) de la variable.
  ArrayShape fullShape() const { return m_underlying_var.trueData()->shape(); }

 protected:

  void updateFromInternal() override
  {
    const Int32 nb_rank = Extents::rank();
    ArrayShape shape_with_item;
    shape_with_item.setNbDimension(nb_rank);
    m_underlying_var.fillShape(shape_with_item);

    ArrayExtents<Extents> new_extents = ArrayExtentsBase<Extents>::fromSpan(shape_with_item.dimensions());
    m_mdspan = MDSpanType(m_underlying_var.trueData()->view().data(), new_extents);
  }

 protected:

  Arcane::Impl::MeshMDVariableRefWrapperT<ItemType, DataType> m_underlying_var;
  MDSpanType m_mdspan;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Classe gérant une variable multi-dimension sur une entité du maillage.
 *
 * \warning Vous devez appeler la méthode reshape() avant d'utiliser ce type de variables.
 *
 * Pour plus d'informations, voir \ref arcanedoc_core_types_axl_md_variable_use.
 */
template <typename ItemType, typename DataType, typename Extents>
class MeshMDVariableRefT
: public MeshMDVariableRefBaseT<ItemType, DataType, typename Extents::template AddedFirstExtentsType<DynExtent>>
{
  // To access m_mdspan
  friend class Arcane::Accelerator::MeshMDVariableInView<ItemType, DataType, Extents>;
  friend class Arcane::Accelerator::MeshMDVariableInOutView<ItemType, DataType, Extents>;

  using AddedFirstExtentsType = typename Extents::template AddedFirstExtentsType<DynExtent>;
  using BasicType = typename DataTypeTraitsT<DataType>::BasicType;
  static_assert(Extents::rank() >= 0 && Extents::rank() <= 3, "Only Extents of rank 0, 1, 2 or 3 are implemented");
  static_assert(std::is_same_v<DataType, BasicType>, "DataType should be a basic type (Real, Int32, Int64, ... )");

 public:

  using BaseClass = MeshMDVariableRefBaseT<ItemType, DataType, AddedFirstExtentsType>;
  using ItemLocalIdType = typename ItemType::LocalIdType;
  static constexpr int nb_dynamic = Extents::nb_dynamic;

 public:

  explicit MeshMDVariableRefT(const VariableBuildInfo& b)
  : BaseClass(b)
  {}

 public:

  DataType& operator()(ItemLocalIdType id) requires(Extents::rank() == 0)
  {
    return this->m_mdspan(id.localId());
  }

  const DataType& operator()(ItemLocalIdType id) const requires(Extents::rank() == 0)
  {
    return this->m_mdspan(id.localId());
  }

  DataType& operator()(ItemLocalIdType id, Int32 i1) requires(Extents::rank() == 1)
  {
    return this->m_mdspan(id.localId(), i1);
  }

  const DataType& operator()(ItemLocalIdType id, Int32 i1) const requires(Extents::rank() == 1)
  {
    return this->m_mdspan(id.localId(), i1);
  }

  DataType& operator()(ItemLocalIdType id, Int32 i1, Int32 i2)
  requires(Extents::rank() == 2)
  {
    return this->m_mdspan(id.localId(), i1, i2);
  }

  const DataType& operator()(ItemLocalIdType id, Int32 i1, Int32 i2) const
  requires(Extents::rank() == 2)
  {
    return this->m_mdspan(id.localId(), i1, i2);
  }

  DataType& operator()(ItemLocalIdType id, Int32 i, Int32 j, Int32 k)
  requires(Extents::rank() == 3)
  {
    return this->m_mdspan(id.localId(), i, j, k);
  }

  const DataType& operator()(ItemLocalIdType id, Int32 i, Int32 j, Int32 k) const
  requires(Extents::rank() == 3)
  {
    return this->m_mdspan(id.localId(), i, j, k);
  }

  /*!
   * \brief Change la forme de la donnée.
   *
   * Le nombre d'éléments de \a dims doit correspondre aux nombre de valeurs
   * dynamiques de \a Extents.
   */
  void reshape(std::array<Int32, Extents::nb_dynamic> dims)
  {
    ArrayShape shape(dims);
    this->m_underlying_var.resizeAndReshape(shape);
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
