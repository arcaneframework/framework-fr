// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* MDDim.h                                                     (C) 2000-2026 */
/*                                                                           */
/* Tag pour les tableaux N-dimensions.                                       */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_MDDIM_H
#define ARCCORE_BASE_MDDIM_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/ExtentsV.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

//! Constante pour un tableau dynamique de rang 0
using MDDim0 = ExtentsV<Int32>;

//! Constante pour un tableau dynamique de rang 1
using MDDim1 = ExtentsV<Int32, DynExtent>;

//! Constante pour un tableau dynamique de rang 2
using MDDim2 = ExtentsV<Int32, DynExtent, DynExtent>;

//! Constante pour un tableau dynamique de rang 3
using MDDim3 = ExtentsV<Int32, DynExtent, DynExtent, DynExtent>;

//! Constante pour un tableau dynamique de rang 4
using MDDim4 = ExtentsV<Int32, DynExtent, DynExtent, DynExtent, DynExtent>;

//! Extent dynamique 1D avec un type d'index spécifique
template<typename IndexType_> using MDDim1Ext = ExtentsV<IndexType_, DynExtent>;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

template <typename IndexType_>
class MDDimType<0, IndexType_>
{
 public:

  using DimType = ExtentsV<IndexType_>;
};
template <typename IndexType_>
class MDDimType<1, IndexType_>
{
 public:

  using DimType = ExtentsV<IndexType_, DynExtent>;
};
template <typename IndexType_>
class MDDimType<2, IndexType_>
{
 public:

  using DimType = ExtentsV<IndexType_, DynExtent, DynExtent>;
};
template <typename IndexType_>
class MDDimType<3, IndexType_>
{
 public:

  using DimType = ExtentsV<IndexType_, DynExtent, DynExtent, DynExtent>;
};
template <typename IndexType_>
class MDDimType<4, IndexType_>
{
 public:

  using DimType = ExtentsV<IndexType_, DynExtent, DynExtent, DynExtent, DynExtent>;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
