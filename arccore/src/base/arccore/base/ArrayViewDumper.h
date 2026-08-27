// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* ArrayViewDumper.h                                           (C) 2000-2026 */
/*                                                                           */
/* Fonctions pour afficher les valeurs des vues de tableaux Arccore          */
/* (ArrayView, Span, ...)                                                    */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_ARRAYVIEWDUMPER_H
#define ARCCORE_BASE_ARRAYVIEWDUMPER_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/ArccoreGlobal.h"

#include <iosfwd>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane::Impl
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Classe utilitaire pour afficher une vue de tableau sur un flux.
 *
 * La méthode \a dumpArray() est générique par rapport au type de flux, nous n'avons donc pas
 * besoin d'inclure l'en-tête 'iostream' dans ce fichier. L'objectif est de réduire
 * le temps de compilation. L'utilisateur doit inclure 'iostream' s'il a besoin d'afficher le tableau.
 */
template <typename ViewType>
class ArrayViewDumper
{
 public:

  template <typename Stream> static void
  dumpArray(Stream& o, ViewType val, int max_print)
  {
    using size_type = typename ViewType::size_type;
    size_type n = val.size();
    if (max_print > 0 && n > max_print) {
      // Affiche uniquement le premier (max_print/2) et le dernier (max_print/2)
      // sinon, si le tableau est très grand, il peut générer des listes de sortie énormes.
      size_type z = (max_print / 2);
      size_type z2 = n - z;
      o << "[0]=\"" << val[0] << '"';
      for (size_type i = 1; i < z; ++i)
        o << " [" << i << "]=\"" << val[i] << '"';
      o << " ... ... (skipping indexes " << z << " to " << z2 << " ) ... ... ";
      for (size_type i = (z2 + 1); i < n; ++i)
        o << " [" << i << "]=\"" << val[i] << '"';
    }
    else {
      for (size_type i = 0; i < n; ++i) {
        if (i != 0)
          o << ' ';
        o << "[" << i << "]=\"" << val[i] << '"';
      }
    }
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane::Impl

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
