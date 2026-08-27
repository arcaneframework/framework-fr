// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* NumMatrixDataView.h                                         (C) 2000-2026 */
/*                                                                           */
/* Implémentation spécifique de DataView (Setter/Getter) pour NumMatrix.     */
/*---------------------------------------------------------------------------*/
#ifndef ARCANE_UTILS_NUMMATRIXDATAVIEW_H
#define ARCANE_UTILS_NUMMATRIXDATAVIEW_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/utils/NumMatrix.h"

#include "arccore/common/DataView.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue en lecture seule pour un NumMatrix<DataType_,Row,Column>.
 */
template <typename DataType_, int Row, int Column>
class NumMatrixDataViewGetter
{
 public:

  //! Type de la matrice
  using NumMatrixType = NumMatrix<DataType_, Row, Column>;
  //! Accesseur pour la matrice
  using AccessorReturnType = const NumMatrixType&;
  //! Accesseur pour un élément de la matrice
  using MatrixElemenAccessor = DataViewGetter<DataType_>;

 public:

  explicit ARCCORE_HOST_DEVICE NumMatrixDataViewGetter(const NumMatrixType* ptr)
  : m_ptr(ptr)
  {}

 public:

  static constexpr ARCCORE_HOST_DEVICE AccessorReturnType build(const NumMatrixType* ptr)
  {
    return { *ptr };
  }

 public:

  constexpr operator AccessorReturnType() const noexcept { return *m_ptr; }

 private:

  const NumMatrixType* m_ptr = nullptr;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Vue modifiable pour un NumMatrix<DataType_,Row,Column>.
 */
template <typename DataType_, int Row, int Column>
class NumMatrixDataViewGetterSetter
: public DataViewGetterSetter<NumMatrix<DataType_, Row, Column>>
{
  using BaseClass = DataViewGetterSetter<NumMatrix<DataType_, Row, Column>>;

 public:

  //! Type de la matrice
  using NumMatrixType = NumMatrix<DataType_, Row, Column>;
  //! Accesseur pour un élément de la matrice
  using MatrixElemenAccessor = DataViewGetterSetter<DataType_>;

 public:

  explicit ARCCORE_HOST_DEVICE NumMatrixDataViewGetterSetter(NumMatrixType* ptr)
  : BaseClass(ptr)
  {}

 public:

  NumMatrixDataViewGetterSetter& operator=(const NumMatrixType& v)
  {
    BaseClass::operator=(v);
    return (*this);
  }

  void fill(const DataType_& v)
  {
    this->m_ptr->fill(v);
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
