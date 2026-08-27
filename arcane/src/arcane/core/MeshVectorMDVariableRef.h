// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* MeshVectorMDVariableRef.h                                   (C) 2000-2026 */
/*                                                                           */
/* Variable 'NumVector' multidimensionnelle sur une entité de maillage.      */
/*---------------------------------------------------------------------------*/
#ifndef ARCANE_CORE_MESHVECTORMDVARIABLEREF_H
#define ARCANE_CORE_MESHVECTORMDVARIABLEREF_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/utils/NumVector.h"

#include "arcane/core/DataView.h"
#include "arcane/core/MeshMDVariableRef.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Classe gérant une variable de type NumVector multidimensionnelle sur une entité de maillage.
 *
 * \warning Vous devez appeler la méthode reshape() avant d'utiliser ce type de variables.
 *
 * Pour plus d'informations, voir \ref arcanedoc_core_types_axl_md_variable_use.
 */
template <typename ItemType, typename DataType, int Size, typename Extents>
class MeshVectorMDVariableRefT
: public MeshMDVariableRefBaseT<ItemType, DataType, typename Extents::template AddedFirstLastExtentsType<DynExtent, Size>>
{
 public:

  using NumVectorType = NumVector<DataType, Size>;

 private:

  using BasicType = typename DataTypeTraitsT<DataType>::BasicType;
  using AddedFirstLastExtentsType = typename Extents::template AddedFirstLastExtentsType<DynExtent, Size>;
  using AddedFirstExtentsType = typename Extents::template AddedFirstExtentsType<DynExtent>;
  using BaseClass = MeshMDVariableRefBaseT<ItemType, DataType, AddedFirstLastExtentsType>;
  static_assert(Extents::rank() >= 0 && Extents::rank() <= 2, "Only Extents of rank 0, 1 or 2 are implemented");
  static_assert(std::is_same_v<DataType, BasicType>, "DataType should be a basic type (Real, Int32, Int64, ... )");

 public:

  using ItemLocalIdType = typename ItemType::LocalIdType;
  using ReferenceType = DataViewGetterSetter<NumVectorType>;
  using ConstReferenceType = DataViewGetter<NumVectorType>;
  using MDSpanType = MDSpan<NumVectorType, AddedFirstExtentsType, RightLayout>;
  static constexpr int nb_dynamic = Extents::nb_dynamic;

 public:

  explicit MeshVectorMDVariableRefT(const VariableBuildInfo& b)
  : BaseClass(b)
  {}

 public:

  //! \name Opérations pour la variable de dimension MDDim0
  ///@{
  //! Accède aux données pour la lecture/écriture
  ReferenceType operator()(ItemLocalIdType id)
  requires(Extents::rank() == 0)
  {
    return ReferenceType(m_vector_mdspan.ptrAt(id.localId()));
  }

  //! Accède aux données en lecture
  ConstReferenceType operator()(ItemLocalIdType id) const
  requires(Extents::rank() == 0)
  {
    return ConstReferenceType(m_vector_mdspan.ptrAt(id.localId()));
  }
  ///@}

  //! \name Opérations pour la variable de dimension MDDim1
  ///@{
  //! Accède aux données pour la lecture/écriture
  ReferenceType operator()(ItemLocalIdType id, Int32 i1)
  requires(Extents::rank() == 1)
  {
    return ReferenceType(m_vector_mdspan.ptrAt(id.localId(), i1));
  }

  //! Accède aux données en lecture
  ConstReferenceType operator()(ItemLocalIdType id, Int32 i1) const
  requires(Extents::rank() == 1)
  {
    return ConstReferenceType(m_vector_mdspan.ptrAt(id.localId(), i1));
  }
  ///@}

  //! \name Opérations pour la variable de dimension MDDim2
  ///@{
  //! Accède aux données pour la lecture/écriture
  ReferenceType operator()(ItemLocalIdType id, Int32 i1, Int32 i2)
  requires(Extents::rank() == 2)
  {
    return ReferenceType(m_vector_mdspan.ptrAt(id.localId(), i1, i2));
  }

  //! Accède aux données en lecture
  ConstReferenceType operator()(ItemLocalIdType id, Int32 i1, Int32 i2) const
  requires(Extents::rank() == 2)
  {
    return ConstReferenceType(m_vector_mdspan.ptrAt(id.localId(), i1, i2));
  }
  ///@}

  /*!
   * \brief Change la forme des données.
   *
   * Le nombre d'éléments dans \a dims doit correspondre au nombre de valeurs dynamiques
   * dans \a Extents.
   */
  void reshape(std::array<Int32, Extents::nb_dynamic> dims)
  {
    std::array<Int32, nb_dynamic + 1> full_dims;
    // Nous ajoutons 'Size' à la fin des dimensions.
    for (int i = 0; i < nb_dynamic; ++i)
      full_dims[i] = dims[i];
    full_dims[nb_dynamic] = Size;
    ArrayShape shape(full_dims);
    this->m_underlying_var.resizeAndReshape(shape);
  }

 protected:

  void updateFromInternal() override
  {
    BaseClass::updateFromInternal();
    // Positionne la valeur de m_vector_mdspan.
    // Elle aura les mêmes dimensions que m_mdspan, sauf que nous
    // retirons la dernière dimension et changeons le type
    // de 'DataType' à 'NumVector<DataType,Size>'.
    DataType* v = this->m_mdspan.to1DSpan().data();
    NumVectorType* nv = reinterpret_cast<NumVectorType*>(v);
    m_vector_mdspan = MDSpanType(nv, this->m_mdspan.extents().dynamicExtents());
  }

 private:

  MDSpanType m_vector_mdspan;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
