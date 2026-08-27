// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* IMeshSection.h                                              (C) 2000-2026 */
/*                                                                           */
/* Interface de service permettant la création d'un maillage avec une        */
/* section d'un autre maillage.                                              */
/*---------------------------------------------------------------------------*/
#ifndef ARCANE_CORE_IMESHSECTION_H
#define ARCANE_CORE_IMESHSECTION_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/core/ArcaneTypes.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*!
 * \brief Interface de service permettant la création d'un maillage avec une section d'un autre maillage.
 */
class ARCANE_CORE_EXPORT IMeshSection
{
 public:

  //! Libère les ressources
  virtual ~IMeshSection() = default;

 public:

  /*!
   * \brief Méthode permettant d'ajouter un plan au service de coupe. L'utilisation de ces plans dépend du service.
   *
   * \param p0 Point du plan.
   * \param normal Normale du plan.
   */
  virtual void addPlane(const Real3& p0, const Real3& normal) = 0;

  /*!
   * \brief Méthode permettant d'ajouter un ensemble de variables à copier sur le nouveau maillage.
   *
   * \param variables Un ensemble de variables sur le maillage original.
   */
  virtual void setVariables(VariableCollection variables) = 0;

  /*!
   * \brief Méthode permettant d'obtenir un ensemble de variables copiées sur le nouveau maillage.
   *
   * \return Un ensemble de variables sur le maillage cloné.
   */
  virtual VariableCollection variables() = 0;

  /*!
   * \brief Méthode permettant de définir un identifiant unique pour créer plusieurs services de section pour un maillage.
   *
   * \param unique_id Identifiant unique du maillage à appliquer.
   */
  virtual void setServiceMeshUniqueId(Int32 unique_id) = 0;

  /*!
   * \brief Méthode permettant de mettre à jour la section du maillage avec tous les plans.
   *
   * Si un appel précédent a modifié le maillage, toutes les mailles seront détruites avant la mise à jour.
   */
  virtual void updateSection() = 0;

  /*!
   * \brief Méthode permettant d'obtenir la section du maillage.
   *
   * \return La section du maillage.
   */
  virtual MeshHandle meshSection() = 0;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
