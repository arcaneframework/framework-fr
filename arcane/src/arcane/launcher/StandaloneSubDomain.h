// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* StandaloneSubDomain.h                                       (C) 2000-2026 */
/*                                                                           */
/* Implémentation autonome d'un sous-domaine.                                */
/*---------------------------------------------------------------------------*/
#ifndef ARCANE_LAUNCHER_STANDALONESUBDOMAIN_H
#define ARCANE_LAUNCHER_STANDALONESUBDOMAIN_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/launcher/LauncherGlobal.h"

#include "arcane/utils/Ref.h"
#include "arcane/core/ArcaneTypes.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*!
 * \brief Implémentation autonome d'un sous-domaine.
 *
 * L'instance de cette classe doit être créée par
 * ArcaneLauncher::createStandaloneSubDomain().
 *
 * Une seule instance est autorisée.
 *
 * Cette classe utilise une sémantique par référence.
 */
class ARCANE_LAUNCHER_EXPORT StandaloneSubDomain
{
  friend class ArcaneLauncher;
  class Impl;

 public:

  //! Constructeur non initialisé.
  StandaloneSubDomain();

 public:

  //! Gestionnaire de trace associé.
  ITraceMng* traceMng();

  //! Sous-domaine.
  ISubDomain* subDomain();

 private:

  Ref<Impl> m_p;

 private:

  void _checkIsInitialized();

 private:

  // Pour ArcaneLauncher.
  void _initUniqueInstance(const String& case_file_name, Span<const std::byte> file_content);
  bool _isValid();
  static void _notifyRemoveStandaloneSubDomain();
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
