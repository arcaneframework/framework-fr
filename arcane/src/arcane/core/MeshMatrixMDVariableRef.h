// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// Voir le fichier COPYRIGHT de niveau supérieur pour les détails.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* MeshMatrixMDVariableRef.h                                   (C) 2000-2026 */
/*                                                                           */
/* Variable 'NumMatrix' multidimensionnelle sur une entité de maillage.      */
/*---------------------------------------------------------------------------*/
#ifndef ARCANE_CORE_MESHMATRIXMDVARIABLEREF_H
#define ARCANE_CORE_MESHMATRIXMDVARIABLEREF_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/utils/NumMatrix.h"
#include "arcane/utils/NumMatrixDataView.h"

#include "arcane/core/MeshMDVariableRef.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Classe gérant une variable de type NumMatrix multidimensionnelle sur une entité de maillage.
 *
 * La dimension de la matrice est fixe et est donnée par (Ligne, Colonne).
 *
 * \warning Vous devez appeler la méthode reshape() avant d'utiliser ce type de variables.
 *
 * Pour plus d'informations, voir \ref arcanedoc_core_types_axl_md_variable_use.
 */
template <typename ItemType, typename DataType_, int Row, int Column, typename Extents>
class MeshMatrixMDVariableRefT
: public MeshMDVariableRefBaseT<ItemType, DataType_, typename Extents::template AddedFirstLastLastExtentsType<DynExtent, Row, Column>>
{
  // To access m_matrix_mdspan
  friend class Arcane::Accelerator::MeshMatrixMDVariableInOutView<ItemType, DataType_, Row, Column, Extents>;
  friend class Arcane::Accelerator::MeshMatrixMDVariableInView<ItemType, DataType_, Row, Column, Extents>;

 public:

  using DataType = DataType_;
  using NumMatrixType = NumMatrix<DataType, Row, Column>;

 private:

  using BasicType = typename DataTypeTraitsT<DataType>::BasicType;
  using AddedFirstLastLastExtentsType = typename Extents::template AddedFirstLastLastExtentsType<DynExtent, Row, Column>;
  using AddedFirstExtentsType = typename Extents::template AddedFirstExtentsType<DynExtent>;
  using BaseClass = MeshMDVariableRefBaseT<ItemType, DataType, AddedFirstLastLastExtentsType>;
  static_assert(Extents::rank() >= 0 && Extents::rank() <= 1, "Only Extents of rank 0 or 1 are implemented");
  static_assert(std::is_same_v<DataType, BasicType>, "DataType should be a basic type (Real, Int32, Int64, ... )");

 public:

  using ItemLocalIdType = typename ItemType::LocalIdType;
  using ReferenceType = NumMatrixDataViewGetterSetter<DataType, Row, Column>;
  using ConstReferenceType = NumMatrixDataViewGetter<DataType, Row, Column>;
  using MDSpanType = MDSpan<NumMatrixType, AddedFirstExtentsType, RightLayout>;
  static constexpr int nb_dynamic = Extents::nb_dynamic;

 public:

  explicit MeshMatrixMDVariableRefT(const VariableBuildInfo& b)
  : BaseClass(b)
  {}

 public:

  //! \name Opérations pour la variable de dimension MDDim0
  ///@{
  //! Vue mutable de la matrice pour l'élément \a id
  ReferenceType operator()(ItemLocalIdType id) requires(Extents::rank() == 0)
  {
    return ReferenceType(m_matrix_mdspan.ptrAt(id.localId()));
  }

  //! Vue en lecture seule de la matrice pour l'élément \a id
  ConstReferenceType operator()(ItemLocalIdType id) const requires(Extents::rank() == 0)
  {
    return ConstReferenceType(m_matrix_mdspan.ptrAt(id.localId()));
  }

  //! Vue mutable de l'élément (i,j) de la matrice pour l'élément \a id
  DataType& operator()(ItemLocalIdType id, Int32 i, Int32 j) requires(Extents::rank() == 0)
  {
    return m_matrix_mdspan(id.localId())(i, j);
  }

  //! Vue en lecture seule de l'élément (i,j) de la matrice pour l'élément \a id
  DataType operator()(ItemLocalIdType id, Int32 i, Int32 j) const requires(Extents::rank() == 0)
  {
    return m_matrix_mdspan(id.localId())(i, j);
  }
  ///@}

  //! \name Opérations pour la variable de dimension MDDim1
  ///@{
  //! Vue mutable de la matrice de l'indice \a index pour l'élément \a id
  ReferenceType operator()(ItemLocalIdType id, Int32 index)
  requires(Extents::rank() == 1)
  {
    return ReferenceType(m_matrix_mdspan.ptrAt(id.localId(), index));
  }

  //! Vue en lecture seule de la matrice de l'indice \a index pour l'élément \a id
  ConstReferenceType operator()(ItemLocalIdType id, Int32 index) const
  requires(Extents::rank() == 1)
  {
    return ConstReferenceType(m_matrix_mdspan.ptrAt(id.localId(), index));
  }

  //! Vue mutable de l'élément (i,j) de la matrice pour l'élément \a id et l'indice \a index
  DataType& operator()(ItemLocalIdType id, Int32 index, Int32 i, Int32 j)
  requires(Extents::rank() == 1)
  {
    return m_matrix_mdspan(id.localId(), index)(i, j);
  }

  //! Vue en lecture seule de l'élément (i,j) de la matrice pour l'élément \a id et l'indice \a index
  DataType operator()(ItemLocalIdType id, Int32 index, Int32 i, Int32 j) const
  requires(Extents::rank() == 1)
  {
    return m_matrix_mdspan(id.localId(), index)(i, j);
  }
  ///@}

  /*!
   * \brief Modifie la forme des données.
   *
   * Le nombre d'éléments dans \a dims doit correspondre au nombre de valeurs dynamiques
   * dans \a Extents.
   */
  void reshape(std::array<Int32, Extents::nb_dynamic> dims)
  {
    std::array<Int32, nb_dynamic + 2> full_dims;
    // We add 'Row' and 'Column' to the end of the dimensions.
    for (int i = 0; i < nb_dynamic; ++i)
      full_dims[i] = dims[i];
    full_dims[nb_dynamic] = Row;
    full_dims[nb_dynamic + 1] = Column;
    ArrayShape shape(full_dims);
    this->m_underlying_var.resizeAndReshape(shape);
  }

 protected:

  void updateFromInternal() override
  {
    BaseClass::updateFromInternal();
    // Positionne la valeur de m_vector_mdspan.
    // Il aura les mêmes dimensions que m_mdspan, sauf que nous
    // retirons la dernière dimension et changeons le type
    // de 'DataType' à 'NumMatrix<DataType,Row,Column>'.
    DataType* v = this->m_mdspan.to1DSpan().data();
    NumMatrixType* nv = reinterpret_cast<NumMatrixType*>(v);
    m_matrix_mdspan = MDSpanType(nv, this->m_mdspan.extents().dynamicExtents());
  }

 private:

  MDSpanType m_matrix_mdspan;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
