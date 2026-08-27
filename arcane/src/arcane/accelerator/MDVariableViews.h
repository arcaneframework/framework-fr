// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* MDVariableViews.h                                           (C) 2000-2026 */
/*                                                                           */
/* Gestion de vues de variables multidimensionnelles pour les accélérateurs. */
/*---------------------------------------------------------------------------*/
#ifndef ARCANE_ACCELERATOR_MDVARIABLEVIEWS_H
#define ARCANE_ACCELERATOR_MDVARIABLEVIEWS_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/utils/ArrayLayout.h"

#include "arcane/accelerator/VariableViews.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane::Accelerator
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Classe de base pour les vues de variables multidimensionnelles sur un élément de maillage.
 */
template <typename ItemType_, typename Accessor_, typename Extents_>
class MeshMDVariableViewBase
{
  using Accessor = Accessor_;
  using AddedFirstExtentsType = typename Extents_::template AddedFirstExtentsType<DynExtent>;

 public:

  using ItemType = ItemType_;
  using DataType = Accessor::ValueType;
  using AccessorReturnType = Accessor::AccessorReturnType;
  using Extents = Extents_;
  using ItemLocalIdType = ItemType::LocalIdType;

 protected:

  using MDSpanType = MDSpan<DataType, AddedFirstExtentsType, RightLayout>;

 protected:

  explicit constexpr ARCCORE_HOST_DEVICE MeshMDVariableViewBase(const MDSpanType& v)
  : m_mdspan(v)
  {
  }

 public:

  constexpr ARCCORE_HOST_DEVICE AccessorReturnType operator()(ItemLocalIdType id) const
  requires(Extents::rank() == 0)
  {
    return Accessor::build(m_mdspan.ptrAt(id.localId()));
  }
  constexpr ARCCORE_HOST_DEVICE AccessorReturnType operator()(ItemLocalIdType id, Int32 i1) const
  requires(Extents::rank() == 1)
  {
    return Accessor::build(m_mdspan.ptrAt(id.localId(), i1));
  }

  constexpr ARCCORE_HOST_DEVICE AccessorReturnType operator()(ItemLocalIdType id, Int32 i1, Int32 i2) const
  requires(Extents::rank() == 2)
  {
    return Accessor::build(m_mdspan.ptrAt(id.localId(), i1, i2));
  }

  constexpr ARCCORE_HOST_DEVICE AccessorReturnType operator()(ItemLocalIdType id, Int32 i, Int32 j, Int32 k) const
  requires(Extents::rank() == 3)
  {
    return Accessor::build(m_mdspan.ptrAt(id.localId(), i, j, k));
  }

 private:

  MDSpanType m_mdspan;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture seule de variable multidimensionnelle sur un élément de maillage.
 */
template <typename ItemType_, typename DataType_, typename Extents>
class MeshMDVariableInView
: protected MeshMDVariableViewBase<ItemType_, DataViewGetter<DataType_>, Extents>
{
  using BaseClass = MeshMDVariableViewBase<ItemType_, DataViewGetter<DataType_>, Extents>;

 public:

  using ItemType = ItemType_;
  using DataType = DataType_;
  using ExtentsType = Extents;
  using ItemLocalIdType = ItemType::LocalIdType;
  using VariableRefType = MeshMDVariableRefT<ItemType, DataType, Extents>;

  using BaseClass::operator();

 private:

  using MDSpanType = VariableRefType::MDSpanType;

 public:

  MeshMDVariableInView(const ViewBuildInfo& view_bi, const VariableRefType& var_ref)
  : BaseClass(var_ref.m_mdspan)
  {
    IVariable* var = var_ref.underlyingVariable().variable();
    VariableViewBase vb(view_bi, var);
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture/écriture de variable multidimensionnelle sur un élément de maillage.
 */
template <typename ItemType_, typename DataType_, typename Extents>
class MeshMDVariableInOutView
: protected MeshMDVariableViewBase<ItemType_, DataViewGetterSetter<DataType_>, Extents>
{
  using BaseClass = MeshMDVariableViewBase<ItemType_, DataViewGetterSetter<DataType_>, Extents>;

 public:

  using ItemType = ItemType_;
  using DataType = DataType_;
  using ExtentsType = Extents;
  using ItemLocalIdType = ItemType::LocalIdType;
  using VariableRefType = MeshMDVariableRefT<ItemType, DataType, Extents>;

  using BaseClass::operator();

 private:

  using MDSpanType = VariableRefType::MDSpanType;

 public:

  MeshMDVariableInOutView(const ViewBuildInfo& view_bi, VariableRefType& var_ref)
  : BaseClass(var_ref.m_mdspan)
  {
    IVariable* var = var_ref.underlyingVariable().variable();
    VariableViewBase vb(view_bi, var);
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Classe de base pour les vues de variables matricielles multidimensionnelles sur un élément de maillage.
 *
 * \a MatrixAccessor_ doit être NumMatrixDataViewGetter ou NumMatrixDataViewGetterSetter
 */
template <typename ItemType_, typename MatrixAccessor_, typename Extents>
class MeshMatrixMDVariableViewBase
{
  using AddedFirstExtentsType = typename Extents::template AddedFirstExtentsType<DynExtent>;

 public:

  using ItemType = ItemType_;
  using MatrixAccessor = MatrixAccessor_;
  using MatrixElementAccessor = MatrixAccessor_::MatrixElemenAccessor;
  using NumMatrixType = MatrixAccessor::NumMatrixType;
  using MatrixAccessorReturnType = MatrixAccessor::AccessorReturnType;
  using MatrixElementAccessorReturnType = MatrixElementAccessor::AccessorReturnType;
  using ItemLocalIdType = ItemType::LocalIdType;

 private:

  using MDSpanType = MDSpan<NumMatrixType, AddedFirstExtentsType, RightLayout>;

 public:

  MeshMatrixMDVariableViewBase(const MDSpanType& matrix_mdspan)
  : m_matrix_mdspan(matrix_mdspan)
  {
  }

 public:

  //! \name Operations for variable of dimension MDDim0
  ///@{

  //! Accesseur de la matrice pour l'élément \a id
  constexpr ARCCORE_HOST_DEVICE MatrixAccessorReturnType operator()(ItemLocalIdType id) const
  requires(Extents::rank() == 0)
  {
    return MatrixAccessor::build(m_matrix_mdspan.ptrAt(id.localId()));
  }

  //! accesseur pour l'élément (i,j) de la matrice pour l'élément \a id
  constexpr ARCCORE_HOST_DEVICE MatrixElementAccessorReturnType operator()(ItemLocalIdType id, Int32 i, Int32 j) const
  requires(Extents::rank() == 0)
  {
    return MatrixElementAccessor::build(&m_matrix_mdspan(id.localId())(i, j));
  }
  ///@}

  //! \name Operations for variable of dimension MDDim1
  //! Accesseur de la matrice d'indice \a index pour l'élément \a id
  constexpr ARCCORE_HOST_DEVICE MatrixAccessorReturnType operator()(ItemLocalIdType id, Int32 index) const
  requires(Extents::rank() == 1)
  {
    return MatrixAccessor::build(m_matrix_mdspan.ptrAt(id.localId(), index));
  }

  //! Accesseur pour l'élément (i,j) de la matrice pour l'élément \a id et l'indice \a index
  constexpr ARCCORE_HOST_DEVICE MatrixElementAccessorReturnType operator()(ItemLocalIdType id, Int32 index, Int32 i, Int32 j) const
  requires(Extents::rank() == 1)
  {
    return MatrixElementAccessor::build(&m_matrix_mdspan(id.localId(), index)(i, j));
  }
  ///@}

 private:

  MDSpanType m_matrix_mdspan;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture seule de variable matricielle multidimensionnelle sur un élément de maillage.
 */
template <typename ItemType_, typename DataType_, int Row, int Column, typename Extents>
class MeshMatrixMDVariableInView
: public MeshMatrixMDVariableViewBase<ItemType_, NumMatrixDataViewGetter<DataType_, Row, Column>, Extents>
{
  using BaseClass = MeshMatrixMDVariableViewBase<ItemType_, NumMatrixDataViewGetter<DataType_, Row, Column>, Extents>;

 public:

  using ItemType = ItemType_;
  using DataType = DataType_;
  using ItemLocalIdType = ItemType::LocalIdType;
  using ConstReferenceType = NumMatrixDataViewGetter<DataType, Row, Column>;
  using VariableRefType = MeshMatrixMDVariableRefT<ItemType, DataType, Row, Column, Extents>;

 private:

  using MDSpanType = VariableRefType::MDSpanType;

 public:

  MeshMatrixMDVariableInView(const ViewBuildInfo& view_bi, const VariableRefType& var_ref)
  : BaseClass(var_ref.m_matrix_mdspan)
  {
    IVariable* var = var_ref.underlyingVariable().variable();
    VariableViewBase vvb(view_bi, var);
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture/écriture de variable matricielle multidimensionnelle sur un élément de maillage.
 */
template <typename ItemType_, typename DataType_, int Row, int Column, typename Extents>
class MeshMatrixMDVariableInOutView
: public MeshMatrixMDVariableViewBase<ItemType_, NumMatrixDataViewGetterSetter<DataType_, Row, Column>, Extents>
{
  using BaseClass = MeshMatrixMDVariableViewBase<ItemType_, NumMatrixDataViewGetterSetter<DataType_, Row, Column>, Extents>;

 public:

  using ItemType = ItemType_;
  using DataType = DataType_;
  using ItemLocalIdType = ItemType::LocalIdType;
  using ConstReferenceType = NumMatrixDataViewGetter<DataType, Row, Column>;
  using VariableRefType = MeshMatrixMDVariableRefT<ItemType, DataType, Row, Column, Extents>;

 private:

  using MDSpanType = VariableRefType::MDSpanType;

 public:

  MeshMatrixMDVariableInOutView(const ViewBuildInfo& view_bi, const VariableRefType& var_ref)
  : BaseClass(var_ref.m_matrix_mdspan)
  {
    IVariable* var = var_ref.underlyingVariable().variable();
    VariableViewBase vvb(view_bi, var);
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture seule de variable de maillage matricielle multidimensionnelle
 */
template <typename ItemType, typename DataType, int Row, int Column, typename Extents>
auto viewIn(const ViewBuildInfo& command, const MeshMatrixMDVariableRefT<ItemType, DataType, Row, Column, Extents>& var)
{
  return MeshMatrixMDVariableInView<ItemType, DataType, Row, Column, Extents>(command, var);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture seule de variable de maillage multidimensionnelle
 */
template <typename ItemType, typename DataType, typename Extents>
auto viewIn(const ViewBuildInfo& command, const MeshMDVariableRefT<ItemType, DataType, Extents>& var)
{
  return MeshMDVariableInView<ItemType, DataType, Extents>(command, var);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture/écriture de variable de maillage matricielle multidimensionnelle
 */
template <typename ItemType, typename DataType, int Row, int Column, typename Extents>
auto viewInOut(const ViewBuildInfo& command, const MeshMatrixMDVariableRefT<ItemType, DataType, Row, Column, Extents>& var)
{
  return MeshMatrixMDVariableInOutView<ItemType, DataType, Row,Column, Extents>(command, var);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture/écriture de variable de maillage matricielle multidimensionnelle
 */
template <typename ItemType, typename DataType, typename Extents>
auto viewInOut(const ViewBuildInfo& command, MeshMDVariableRefT<ItemType, DataType, Extents>& var)
{
  return MeshMDVariableInOutView<ItemType, DataType, Extents>(command, var);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arcane::Accelerator

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
