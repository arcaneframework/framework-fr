// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* MaterialVariableViews.h                                     (C) 2000-2026 */
/*                                                                           */
/* Gestion des vues sur les variables matériaux pour les accélérateurs.      */
/*---------------------------------------------------------------------------*/
#ifndef ARCANE_ACCELERATOR_MATERIALVARIABLEVIEWS_H
#define ARCANE_ACCELERATOR_MATERIALVARIABLEVIEWS_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/core/materials/IMeshMaterialVariable.h"
#include "arcane/core/materials/MeshMaterialVariableRef.h"
#include "arcane/core/materials/MeshEnvironmentVariableRef.h"

#include "arcane/accelerator/AcceleratorGlobal.h"
#include "arcane/accelerator/VariableViews.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

using namespace Arcane;
using namespace Arcane::Materials;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane::Accelerator
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Classe de base des vues sur les variables matériaux.
 */
class ARCANE_ACCELERATOR_EXPORT MatVariableViewBase
{
 public:

  // Pour l'instant n'utilise pas encore les paramètres
  MatVariableViewBase(const ViewBuildInfo&, IMeshMaterialVariable*);
  // Pour l'instant n'utilise pas encore les paramètres
  MatVariableViewBase() = default;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture sur une variable scalaire du maillage.
 */
template <typename ItemType, typename DataType>
class MatItemVariableScalarInViewT
: public MatVariableViewBase
{
  // TODO: Faut-il rajouter la gestion des SIMD comme dans ItemVariableScalarInViewT ?

 private:

  using ItemIndexType = typename ItemTraitsT<ItemType>::LocalIdType;

 public:

  MatItemVariableScalarInViewT(const ViewBuildInfo& vbi, IMeshMaterialVariable* var, ArrayView<DataType>* v)
  : MatVariableViewBase(vbi, var)
  , m_value(v)
  {}
  MatItemVariableScalarInViewT() = default;

  //! Opérateur d'accès pour l'entité \a item
  ARCCORE_HOST_DEVICE const DataType& operator[](ComponentItemLocalId lid) const
  {
    return this->m_value[lid.localId().arrayIndex()][lid.localId().valueIndex()];
  }

  //! Opérateur d'accès pour l'entité \a item
  ARCCORE_HOST_DEVICE const DataType& operator[](PureMatVarIndex pmvi) const
  {
    return this->m_value[0][pmvi.valueIndex()];
  }

  //! Surcharge pour accéder à la valeure globale à partir du cell id
  ARCCORE_HOST_DEVICE const DataType& operator[](ItemIndexType item) const
  {
    return this->m_value[0][item.localId()];
  }

  //! Opérateur d'accès pour l'entité \a item
  ARCCORE_HOST_DEVICE const DataType& value(ComponentItemLocalId mvi) const
  {
    return this->m_value[mvi.localId().arrayIndex()][mvi.localId().valueIndex()];
  }

  ARCCORE_HOST_DEVICE const DataType& value0(PureMatVarIndex idx) const
  {
    return this->m_value[0][idx.valueIndex()];
  }

 private:

  ArrayView<DataType>* m_value = nullptr;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en écriture sur une variable scalaire du maillage.
 */
template <typename ItemType, typename Accessor>
class MatItemVariableScalarOutViewT
: public MatVariableViewBase
{
 private:

  using DataType = typename Accessor::ValueType;
  using DataTypeReturnType = DataType&;
  using ItemIndexType = typename ItemTraitsT<ItemType>::LocalIdType;

  // TODO: faut il rajouter des ARCANE_CHECK_AT(mvi.arrayIndex(), m_value.size());
  // il manquera tjrs le check sur l'autre dimension
  // TODO: Faut il rajouter la gestion des types SIMD comme dans ItemVariableScalarOutViewT ?

 public:

  MatItemVariableScalarOutViewT(const ViewBuildInfo& vbi, IMeshMaterialVariable* var, ArrayView<DataType>* v)
  : MatVariableViewBase(vbi, var)
  , m_value(v)
  {}
  MatItemVariableScalarOutViewT() = default;

  //! Opérateur d'accès pour l'entité \a item
  ARCCORE_HOST_DEVICE Accessor operator[](ComponentItemLocalId lid) const
  {
    return Accessor(this->m_value[lid.localId().arrayIndex()].data() + lid.localId().valueIndex());
  }

  ARCCORE_HOST_DEVICE Accessor operator[](PureMatVarIndex pmvi) const
  {
    return Accessor(this->m_value[0][pmvi.valueIndex()]);
  }

  //! Surcharge pour accéder à la valeure globale à partir du cell id
  ARCCORE_HOST_DEVICE Accessor operator[](ItemIndexType item) const
  {
    ARCANE_CHECK_AT(item.localId(), this->m_value[0].size());
    return Accessor(this->m_value[0].data() + item.localId());
  }

  //! Opérateur d'accès pour l'entité \a item
  ARCCORE_HOST_DEVICE Accessor value(ComponentItemLocalId lid) const
  {
    return Accessor(this->m_value[lid.localId().arrayIndex()].data() + lid.localId().valueIndex());
  }

  //! Positionne la valeur pour l'entité \a item à \a v
  ARCCORE_HOST_DEVICE void setValue(ComponentItemLocalId lid, const DataType& v) const
  {
    this->m_value[lid.localId().arrayIndex()][lid.localId().valueIndex()] = v;
  }

  ARCCORE_HOST_DEVICE Accessor value0(PureMatVarIndex idx) const
  {
    return Accessor(this->m_value[0][idx.valueIndex()]);
  }

 private:

  ArrayView<DataType>* m_value = nullptr;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture seule sur une variable constituante de tableau 1D.
 */
template <typename ItemType, typename DataType>
class MatItemVariableArrayInViewT
: public MatVariableViewBase
{
 private:

  using ItemIndexType = typename ItemTraitsT<ItemType>::LocalIdType;

 public:

  using ContainerViewType = MeshMaterialVariableArrayContainerView<const DataType>;

 public:

  MatItemVariableArrayInViewT(const ViewBuildInfo& vbi, IMeshMaterialVariable* var,
                              const ContainerViewType& v)
  : MatVariableViewBase(vbi, var)
  , m_value(v)
  {}
  MatItemVariableArrayInViewT() = default;

  //! Opérateur d'accès pour l'entité \a item
  ARCCORE_HOST_DEVICE SmallSpan<const DataType> operator[](ComponentItemLocalId lid) const
  {
    return m_value[lid];
  }

  //! Opérateur d'accès pour l'entité \a item
  ARCCORE_HOST_DEVICE SmallSpan<const DataType> operator[](PureMatVarIndex pmvi) const
  {
    return m_value[pmvi];
  }

  //! Surcharge pour accéder à la valeur globale à partir de l'identifiant de cellule
  ARCCORE_HOST_DEVICE SmallSpan<const DataType> operator[](ItemIndexType item) const
  {
    return m_value[item];
  }

  //! Opérateur d'accès pour l'entité \a item
  ARCCORE_HOST_DEVICE SmallSpan<const DataType> value(ComponentItemLocalId mvi) const
  {
    return m_value[mvi];
  }

  //! Surcharge pour accéder à la valeur globale à partir de l'identifiant de cellule
  ARCCORE_HOST_DEVICE SmallSpan<const DataType> value(ItemIndexType item) const
  {
    return m_value[item];
  }

 private:

  ContainerViewType m_value;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue d'écriture sur une variable constituante de tableau 1D.
 */
template <typename ItemType, typename Accessor>
class MatItemVariableArrayOutViewT
: public MatVariableViewBase
{
 private:

  using DataType = typename Accessor::ValueType;
  using DataTypeReturnType = DataType&;
  using ItemIndexType = typename ItemTraitsT<ItemType>::LocalIdType;
  using ContainerViewType = MeshMaterialVariableArrayContainerView<DataType>;

 public:

  MatItemVariableArrayOutViewT(const ViewBuildInfo& vbi, IMeshMaterialVariable* var, const ContainerViewType& v)
  : MatVariableViewBase(vbi, var)
  , m_value(v)
  {}
  MatItemVariableArrayOutViewT() = default;

  //! Opérateur d'accès pour l'entité \a item
  ARCCORE_HOST_DEVICE Accessor operator[](ComponentItemLocalId lid) const
  {
    return Accessor(m_value[lid]);
  }

  ARCCORE_HOST_DEVICE Accessor operator[](PureMatVarIndex pmvi) const
  {
    return Accessor(m_value[pmvi]);
  }

  //! Surcharge pour accéder à la valeur globale à partir de l'identifiant de cellule
  ARCCORE_HOST_DEVICE Accessor operator[](ItemIndexType item) const
  {
    return Accessor(m_value[item]);
  }

  //! Opérateur d'accès pour l'entité \a item
  ARCCORE_HOST_DEVICE Accessor value(ComponentItemLocalId lid) const
  {
    return Accessor(m_value[lid]);
  }

 private:

  ContainerViewType m_value;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en écriture pour les variables materiaux scalaires.
 */
template <typename DataType> auto
viewOut(const ViewBuildInfo& vbi, CellMaterialVariableScalarRef<DataType>& var)
{
  using Accessor = DataViewSetter<DataType>;
  return MatItemVariableScalarOutViewT<Cell, Accessor>(vbi, var.materialVariable(), var._internalValue());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue de lecture/écriture pour les variables constituantes de tableau 1D
 */
template <typename DataType> auto
viewOut(const ViewBuildInfo& vbi, CellMaterialVariableArrayRef<DataType>& var)
{
  using Accessor = View1DSetter<DataType>;
  using ViewType = MatItemVariableArrayOutViewT<Cell, Accessor>;
  return ViewType(vbi, var.materialVariable(), var.containerView());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en écriture pour les variables materiaux scalaire
 */
template <typename DataType> auto
viewOut(const ViewBuildInfo& vbi, CellEnvironmentVariableScalarRef<DataType>& var)
{
  using Accessor = DataViewSetter<DataType>;
  return MatItemVariableScalarOutViewT<Cell, Accessor>(vbi, var.materialVariable(), var._internalValue());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue de lecture/écriture pour les variables constituantes de tableau 1D
 */
template <typename DataType> auto
viewOut(const ViewBuildInfo& vbi, CellEnvironmentVariableArrayRef<DataType>& var)
{
  using Accessor = View1DSetter<DataType>;
  using ViewType = MatItemVariableArrayOutViewT<Cell, Accessor>;
  return ViewType(vbi, var.materialVariable(), var.containerView());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture/écriture pour les variables materiaux scalaire
 */
template <typename DataType> auto
viewInOut(const ViewBuildInfo& vbi, CellMaterialVariableScalarRef<DataType>& var)
{
  using Accessor = DataViewGetterSetter<DataType>;
  return MatItemVariableScalarOutViewT<Cell, Accessor>(vbi, var.materialVariable(), var._internalValue());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue de lecture/écriture pour les variables constituantes de tableau 1D
 */
template <typename DataType> auto
viewInOut(const ViewBuildInfo& vbi, CellMaterialVariableArrayRef<DataType>& var)
{
  using Accessor = View1DGetterSetter<DataType>;
  using ViewType = MatItemVariableArrayOutViewT<Cell, Accessor>;
  return ViewType(vbi, var.materialVariable(), var.containerView());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture/écriture pour les variables materiaux scalaire
 */
template <typename DataType> auto
viewInOut(const ViewBuildInfo& vbi, CellEnvironmentVariableScalarRef<DataType>& var)
{
  using Accessor = DataViewGetterSetter<DataType>;
  return MatItemVariableScalarOutViewT<Cell, Accessor>(vbi, var.materialVariable(), var._internalValue());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue de lecture/écriture pour les variables environnementales de tableau 1D
 */
template <typename DataType> auto
viewInOut(const ViewBuildInfo& vbi, CellEnvironmentVariableArrayRef<DataType>& var)
{
  using Accessor = View1DGetterSetter<DataType>;
  using ViewType = MatItemVariableArrayOutViewT<Cell, Accessor>;
  return ViewType(vbi, var.materialVariable(), var.containerView());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture pour les variables materiaux scalaire
 */
template <typename DataType> auto
viewIn(const ViewBuildInfo& vbi, const CellMaterialVariableScalarRef<DataType>& var)
{
  return MatItemVariableScalarInViewT<Cell, DataType>(vbi, var.materialVariable(), var._internalValue());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture pour les variables materiaux de tableau 1D
 */
template <typename DataType> auto
viewIn(const ViewBuildInfo& vbi, const CellMaterialVariableArrayRef<DataType>& var)
{
  return MatItemVariableArrayInViewT<Cell, DataType>(vbi, var.materialVariable(), var.constContainerView());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture pour les variables materiaux scalaires
 */
template <typename DataType> auto
viewIn(const ViewBuildInfo& vbi, const CellEnvironmentVariableScalarRef<DataType>& var)
{
  return MatItemVariableScalarInViewT<Cell, DataType>(vbi, var.materialVariable(), var._internalValue());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture pour les variables materiaux de tableau 1D
 */
template <typename DataType> auto
viewIn(const ViewBuildInfo& vbi, const CellEnvironmentVariableArrayRef<DataType>& var)
{
  return MatItemVariableArrayInViewT<Cell, DataType>(vbi, var.materialVariable(), var.constContainerView());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane::Accelerator

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
