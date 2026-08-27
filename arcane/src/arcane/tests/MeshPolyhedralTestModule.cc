// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* MeshPolyhedralTestModule.cc                                  C) 2000-2026 */
/*                                                                           */
/* Module de test pour maillage personnalisé                                 */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/utils/ValueChecker.h"

#include "arcane/core/ITimeLoopMng.h"
#include "arcane/core/IMesh.h"
#include "arcane/core/MeshHandle.h"
#include "arcane/core/IMeshMng.h"
#include "arcane/core/internal/IMeshInternal.h"
#include "arcane/core/Connectivity.h"
#include "arcane/core/ItemGroup.h"

#include "arcane/mesh/PolyhedralMesh.h"

#include <numeric>

#include "arcane/tests/MeshPolyhedralTest_axl.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace ArcaneTest::MeshPolyhedral
{
using namespace Arcane;
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

class MeshPolyhedralTestModule : public ArcaneMeshPolyhedralTestObject
{
 public:

  explicit MeshPolyhedralTestModule(const ModuleBuildInfo& sbi)
  : ArcaneMeshPolyhedralTestObject(sbi)
  {}

 public:

  void init()
  {
    auto mesh_handle = subDomain()->defaultMeshHandle();
    if (mesh_handle.hasMesh() && !options()->readOnly) {
      info() << "-- Nom du maillage: " << mesh()->name();
      _testKind(mesh());
      _testDimensions(mesh());
      _testCoordinates(mesh());
      _testEnumerationAndConnectivities(mesh());
      _testVariables(mesh());
      _testGroups(mesh());
      _testMeshUtilities(mesh());
      _testMeshModifier(mesh());
      _testConnectivity(mesh());
    }
    else
      info() << "Aucun maillage";

    subDomain()->timeLoopMng()->stopComputeLoop(true);
  }

 private:

  void _testKind(IMesh* mesh);
  void _testEnumerationAndConnectivities(IMesh* mesh);
  void _testVariables(IMesh* mesh);
  void _testGroups(IMesh* mesh);
  void _testDimensions(IMesh* mesh);
  void _testCoordinates(IMesh* mesh);
  void _testMeshUtilities(IMesh* mesh);
  void _testMeshModifier(IMesh* mesh);
  void _testConnectivity(IMesh* mesh);
  void _buildGroup(IItemFamily* family, String const& group_name);
  void _checkBoundaryFaceGroup(IMesh* mesh, String const& boundary_face_group_name) const;
  void _checkInternalFaceGroup(IMesh* mesh, String const& internal_face_group_name) const;
  void _checkFlags(IMesh* mesh) const;
  template <typename VariableRefType>
  void _checkVariable(VariableRefType variable, ItemGroup item_group);
  template <typename VariableRefType>
  void _checkVariableWithRefValue(VariableRefType variable, ItemGroup item_group, const typename VariableRefType::DataType& ref_sum);
  template <typename VariableArrayRefType>
  void _checkArrayVariable(VariableArrayRefType variable, ItemGroup item_group);
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void MeshPolyhedralTestModule::
_testKind(IMesh* mesh)
{
  // Vérifie le type de maillage
  if (mesh->meshKind().meshStructure() != eMeshStructure::Polyhedral) {
    ARCANE_FATAL("Le type de maillage pour le maillage {0} n'est pas eMeshStructure::Polyhedral", mesh->name());
  }
  // Définis le type de maillage (pour le test, fait par le lecteur de maillage)
  // Jusqu'à présent pour PolyhedralMesh doit être : eMeshStructure::Polyhedral, eMeshAMRKind::Node
  MeshKind kind;
  kind.setMeshStructure(eMeshStructure::Polyhedral);
  kind.setMeshAMRKind(eMeshAMRKind::None);
  mesh->_internalApi()->setMeshKind(kind);
  // Finalise la vérification internalApi. Les tests DoF sont effectués dans DoFTester
  auto dof_mng = mesh->_internalApi()->dofConnectivityMng();
  if (!dof_mng)
    ARCANE_FATAL("Impossible d'obtenir DoFConnectivityMng à partir de PolyhedralMesh");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void MeshPolyhedralTestModule::
_testEnumerationAndConnectivities(IMesh* mesh)
{
  info() << "- Test de maillage polyédrique -";
  info() << "- Dimension du maillage " << mesh->dimension();
  info() << "- Nb de mailles " << mesh->nbItem(IK_Cell) << " ou " << mesh->nbCell();
  info() << "- Nb de faces du maillage " << mesh->nbItem(IK_Face) << " ou " << mesh->nbFace();
  info() << "- Nb d'arêtes du maillage " << mesh->nbItem(IK_Edge) << " ou " << mesh->nbEdge();
  info() << "- Nb de nœuds du maillage " << mesh->nbItem(IK_Node) << " ou " << mesh->nbNode();
  info() << "Famille de mailles " << mesh->cellFamily()->name();
  info() << "Famille de nœuds " << mesh->nodeFamily()->name();
  auto all_cells = mesh->allCells();
  // TOUTES LES MAILLES
  ENUMERATE_CELL (icell, all_cells) {
    debug(Trace::High) << "maille avec index " << icell.index();
    debug(Trace::High) << "maille avec lid " << icell.localId();
    debug(Trace::High) << "maille avec uid " << icell->uniqueId().asInt64();
    debug(Trace::High) << "maille nombre de nœuds " << icell->nodes().size();
    debug(Trace::High) << "maille nombre de faces " << icell->faces().size();
    debug(Trace::High) << "maille nombre d'arêtes " << icell->edges().size();
    for (Node node : icell->nodes()) {
      debug(Trace::High) << "nœud maille lid " << node.localId() << " uid " << node.uniqueId().asInt64();
    }
    for (Face face : icell->faces()) {
      debug(Trace::High) << "face maille lid " << face.localId() << " uid " << face.uniqueId().asInt64();
    }
    for (Edge edge : icell->edges()) {
      debug(Trace::High) << "arête maille lid " << edge.localId() << " uid " << edge.uniqueId().asInt64();
    }
  }
  // TOUTES LES FACES
  ENUMERATE_FACE (iface, mesh->allFaces()) {
    debug(Trace::High) << "face avec index " << iface.index();
    debug(Trace::High) << "face avec lid " << iface.localId();
    debug(Trace::High) << "face avec uid " << iface->uniqueId().asInt64();
    debug(Trace::High) << "face nombre de nœuds " << iface->nodes().size();
    debug(Trace::High) << "face nombre de mailles " << iface->cells().size();
    debug(Trace::High) << "face nombre d'arêtes " << iface->edges().size();
    debug(Trace::High) << "maille arrière de la face " << iface->backCell().localId();
    debug(Trace::High) << "maille avant de la face " << iface->frontCell().localId();
    for (Node node : iface->nodes()) {
      debug(Trace::High) << "nœud face lid " << node.localId() << " uid " << node.uniqueId().asInt64();
    }
    auto cell_index = 0;
    bool are_face_cells_ok = true;
    for (Cell cell : iface->cells()) {
      debug(Trace::High) << "maille face lid " << cell.localId() << " uid " << cell.uniqueId().asInt64();
      if (cell_index == 0) {
        if (iface->itemBase().flags() & ItemFlags::II_FrontCellIsFirst)
          are_face_cells_ok = are_face_cells_ok && cell.uniqueId() == iface->frontCell().uniqueId();
        else
          are_face_cells_ok = are_face_cells_ok && cell.uniqueId() == iface->backCell().uniqueId();
      }
      else
        are_face_cells_ok = are_face_cells_ok && cell.uniqueId() == iface->frontCell().uniqueId();
      ++cell_index;
    }
    if (!are_face_cells_ok) {
      ARCANE_FATAL("Problème avec les mailles de la face.");
    }
    for (Edge edge : iface->edges()) {
      debug(Trace::High) << "arête face lid " << edge.localId() << " uid " << edge.uniqueId().asInt64();
    }
    // vérifie les mailles de frontière
    if (iface->cells().size() == 1 && !iface->isSubDomainBoundary()) {
      ARCANE_FATAL("Une face avec une maille est une frontière.");
    }
    if (iface->isSubDomainBoundary()) {
      debug(Trace::High) << "maille de frontière de la face lid " << iface->boundaryCell().localId();
      debug(Trace::High) << "maille de frontière de la face uid " << iface->boundaryCell().uniqueId().asInt64();
      if (iface->boundaryCell().localId() == NULL_ITEM_LOCAL_ID) {
        ARCANE_FATAL("La maille de frontière d'une face est nulle.");
      }
      if (iface->cells().size() > 1) {
        ARCANE_FATAL("Une face de frontière a plus d'une maille.");
      }
    }
  }
  // Vérifie les drapeaux de la face
  _checkFlags(mesh);
  // TOUS LES NŒUDS
  ENUMERATE_NODE (inode, mesh->allNodes()) {
    debug(Trace::High) << "nœud avec index " << inode.index();
    debug(Trace::High) << "nœud avec lid " << inode.localId();
    debug(Trace::High) << "nœud avec uid " << inode->uniqueId().asInt64();
    debug(Trace::High) << "nœud nombre de faces " << inode->faces().size();
    debug(Trace::High) << "nœud nombre de mailles " << inode->cells().size();
    debug(Trace::High) << "nœud nombre d'arêtes " << inode->edges().size();
    for (Face face : inode->faces()) {
      debug(Trace::High) << "face nœud lid " << face.localId() << " uid " << face.uniqueId().asInt64();
    }
    for (Cell cell : inode->cells()) {
      debug(Trace::High) << "maille nœud lid " << cell.localId() << " uid " << cell.uniqueId().asInt64();
    }
    for (Edge edge : inode->edges()) {
      debug(Trace::High) << "arête nœud lid " << edge.localId() << " uid " << edge.uniqueId().asInt64();
    }
  }
  // TOUTES LES ARÊTES
  ENUMERATE_EDGE (iedge, mesh->allEdges()) {
    debug(Trace::High) << "arête avec index " << iedge.index();
    debug(Trace::High) << "arête avec lid " << iedge.localId();
    debug(Trace::High) << "arête avec uid " << iedge->uniqueId().asInt64();
    debug(Trace::High) << "arête nombre de faces " << iedge->faces().size();
    debug(Trace::High) << "arête nombre de mailles " << iedge->cells().size();
    debug(Trace::High) << "arête nombre de nœuds " << iedge->nodes().size();
    for (Face face : iedge->faces()) {
      debug(Trace::High) << "face arête lid " << face.localId() << " uid " << face.uniqueId();
    }
    for (Cell cell : iedge->cells()) {
      debug(Trace::High) << "maille arête lid " << cell.localId() << " uid " << cell.uniqueId();
    }
    for (Node node : iedge->nodes()) {
      debug(Trace::High) << "nœud arête lid " << node.localId() << " uid " << node.uniqueId();
    }
  }
  // Éléments actifs : aucun AMR disponible avec maillage polyédrique mais doit retourner tous les éléments
  bool is_active_ok = (mesh->allActiveCells().size() == mesh->allCells().size());
  is_active_ok &= (mesh->ownActiveCells().size() == mesh->ownCells().size());
  is_active_ok &= (mesh->allActiveFaces().size() == mesh->allFaces().size());
  is_active_ok &= (mesh->ownFaces().size() == mesh->ownFaces().size());
  is_active_ok &= (mesh->innerActiveFaces().size() == mesh->allCells().innerFaceGroup().size());
  is_active_ok &= (mesh->outerActiveFaces().size() == mesh->outerFaces().size());
  is_active_ok &= (mesh->allLevelCells(0).size() == mesh->allCells().size());
  is_active_ok &= (mesh->ownLevelCells(0).size() == mesh->ownCells().size());
  is_active_ok &= (mesh->allLevelCells(1).empty());
  is_active_ok &= (mesh->ownLevelCells(1).empty());
  if (!is_active_ok)
    ARCANE_FATAL("L'implémentation du maillage polyédrique ne gère pas correctement les méthodes activeCells. Doit retourner tous les éléments.");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void MeshPolyhedralTestModule::_testVariables(IMesh* mesh)
{
  // teste les variables
  info() << " -- variables de test -- ";
  // variable de maille
  m_cell_variable.fill(1);
  _checkVariable(m_cell_variable, mesh->allCells());
  // variable de nœud
  m_node_variable.fill(1);
  _checkVariable(m_node_variable, mesh->allNodes());
  // variable de face
  m_face_variable.fill(1);
  _checkVariable(m_face_variable, mesh->allFaces());
  // variable d'arête
  m_edge_variable.fill(1);
  _checkVariable(m_edge_variable, mesh->allEdges());
  // Vérifie les variables définies dans le fichier de maillage
  // Variables de maille
  for (const auto& variable_name : options()->getCheckCellVariableReal()) {
    if (!Arcane::AbstractModule::subDomain()->variableMng()->findMeshVariable(mesh, variable_name))
      ARCANE_FATAL("Impossible de trouver la variable de maillage {0}", variable_name);
    VariableCellReal var{ VariableBuildInfo(mesh, variable_name) };
    _checkVariable(var, mesh->allCells());
  }
  for (const auto& variable_name : options()->getCheckCellVariableInteger()) {
    if (!Arcane::AbstractModule::subDomain()->variableMng()->findMeshVariable(mesh, variable_name))
      ARCANE_FATAL("Impossible de trouver la variable de maillage {0}", variable_name);
    VariableCellInteger var{ VariableBuildInfo(mesh, variable_name) };
    _checkVariable(var, mesh->allCells());
  }
  for (const auto& variable_name : options()->getCheckCellVariableArrayInteger()) {
    if (!Arcane::AbstractModule::subDomain()->variableMng()->findMeshVariable(mesh, variable_name))
      ARCANE_FATAL("Impossible de trouver la variable de maillage {0}", variable_name);
    VariableCellArrayInteger var{ VariableBuildInfo(mesh, variable_name) };
    _checkArrayVariable(var, mesh->allCells());
  }
  for (const auto& variable_name : options()->getCheckCellVariableArrayReal()) {
    if (!Arcane::AbstractModule::subDomain()->variableMng()->findMeshVariable(mesh, variable_name))
      ARCANE_FATAL("Impossible de trouver la variable de maillage {0}", variable_name);
    VariableCellArrayReal var{ VariableBuildInfo(mesh, variable_name) };
    _checkArrayVariable(var, mesh->allCells());
  }
  // Variables de nœud
  for (const auto& variable_name : options()->getCheckNodeVariableReal()) {
    if (!Arcane::AbstractModule::subDomain()->variableMng()->findMeshVariable(mesh, variable_name))
      ARCANE_FATAL("Impossible de trouver la variable de maillage {0}", variable_name);
    VariableNodeReal var{ VariableBuildInfo(mesh, variable_name) };
    _checkVariable(var, mesh->allNodes());
  }
  for (const auto& variable_name : options()->getCheckNodeVariableInteger()) {
    if (!Arcane::AbstractModule::subDomain()->variableMng()->findMeshVariable(mesh, variable_name))
      ARCANE_FATAL("Impossible de trouver la variable de maillage {0}", variable_name);
    VariableNodeInteger var{ VariableBuildInfo(mesh, variable_name) };
    _checkVariable(var, mesh->allNodes());
  }
  for (const auto& variable_name : options()->getCheckNodeVariableArrayInteger()) {
    if (!Arcane::AbstractModule::subDomain()->variableMng()->findMeshVariable(mesh, variable_name))
      ARCANE_FATAL("Impossible de trouver la variable de maillage {0}", variable_name);
    VariableNodeArrayInteger var{ VariableBuildInfo(mesh, variable_name) };
    _checkArrayVariable(var, mesh->allNodes());
  }
  for (const auto& variable_name : options()->getCheckNodeVariableArrayReal()) {
    if (!Arcane::AbstractModule::subDomain()->variableMng()->findMeshVariable(mesh, variable_name))
      ARCANE_FATAL("Impossible de trouver la variable de maillage {0}", variable_name);
    VariableNodeArrayReal var{ VariableBuildInfo(mesh, variable_name) };
    _checkArrayVariable(var, mesh->allNodes());
  }
  // Variables de face
  for (const auto& variable_name : options()->getCheckFaceVariableReal()) {
    if (!Arcane::AbstractModule::subDomain()->variableMng()->findMeshVariable(mesh, variable_name))
      ARCANE_FATAL("Impossible de trouver la variable de maillage {0}", variable_name);
    VariableFaceReal var{ VariableBuildInfo(mesh, variable_name) };
    _checkVariable(var, mesh->allFaces());
  }
  for (const auto& variable_name : options()->getCheckFaceVariableInteger()) {
    if (!Arcane::AbstractModule::subDomain()->variableMng()->findMeshVariable(mesh, variable_name))
      ARCANE_FATAL("Impossible de trouver la variable de maillage {0}", variable_name);
    VariableFaceInteger var{ VariableBuildInfo(mesh, variable_name) };
    _checkVariable(var, mesh->allFaces());
  }
  for (const auto& variable_name : options()->getCheckFaceVariableArrayInteger()) {
    if (!Arcane::AbstractModule::subDomain()->variableMng()->findMeshVariable(mesh, variable_name))
      ARCANE_FATAL("Impossible de trouver la variable de maillage {0}", variable_name);
    VariableFaceArrayInteger var{ VariableBuildInfo(mesh, variable_name) };
    _checkArrayVariable(var, mesh->allFaces());
  }
  for (const auto& variable_name : options()->getCheckFaceVariableArrayReal()) {
    if (!Arcane::AbstractModule::subDomain()->variableMng()->findMeshVariable(mesh, variable_name))
      ARCANE_FATAL("Impossible de trouver la variable de maillage {0}", variable_name);
    VariableFaceArrayReal var{ VariableBuildInfo(mesh, variable_name) };
    _checkArrayVariable(var, mesh->allFaces());
  }
  for (const auto& variable_with_ref_option : options()->checkCellVariableIntegerWithRefValue()) {
    String variable_name = variable_with_ref_option->getVarName();
    if (!Arcane::AbstractModule::subDomain()->variableMng()->findMeshVariable(mesh, variable_name))
      ARCANE_FATAL("Impossible de trouver la variable de maillage {0}", variable_name);
    VariableCellInteger var{ VariableBuildInfo(mesh, variable_name) };
    _checkVariableWithRefValue(var, mesh->allCells(), variable_with_ref_option->getVarRefSum());
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void MeshPolyhedralTestModule::
_testGroups(IMesh* mesh)
{
  // Groupes AllItems
  ARCANE_ASSERT((!mesh->findGroup("AllCells").null()), ("Le groupe AllCells n'a pas été créé"));
  ARCANE_ASSERT((!mesh->findGroup("AllNodes").null()), ("Le groupe AllNodes n'a pas été créé"));
  ARCANE_ASSERT((!mesh->findGroup("AllFaces").null()), ("Le groupe AllFaces n'a pas été créé"));
  ARCANE_ASSERT((!mesh->findGroup("AllEdges").null()), ("Le groupe AllEdges n'a pas été créé"));
  // Groupes OwnItems
  if (!Arcane::AbstractModule::subDomain()->parallelMng()->isParallel()) {
    ValueChecker vc{ A_FUNCINFO };
    vc.areEqual(mesh->allCells().size(), mesh->ownCells().size(), "La taille du groupe All et own pour les mailles diffère en séquentiel.");
    vc.areEqual(mesh->allFaces().size(), mesh->ownFaces().size(), "La taille du groupe All et own pour les faces diffère en séquentiel.");
    vc.areEqual(mesh->allEdges().size(), mesh->ownEdges().size(), "La taille du groupe All et own pour les arêtes diffère en séquentiel.");
    vc.areEqual(mesh->allNodes().size(), mesh->ownNodes().size(), "La taille du groupe All et own pour les nœuds diffère en séquentiel.");
  }
  // Groupe de maille
  String group_name = "my_cell_group";
  _buildGroup(mesh->cellFamily(), group_name);
  ARCANE_ASSERT((!mesh->findGroup(group_name).null()), ("Le groupe my_cell_group n'a pas été créé"));
  PartialVariableCellInt32 partial_cell_var({ mesh, "partial_cell_variable", mesh->cellFamily()->name(), group_name });
  partial_cell_var.fill(1);
  _checkVariable(partial_cell_var, partial_cell_var.itemGroup());
  // Groupe de nœud
  group_name = "my_node_group";
  _buildGroup(mesh->nodeFamily(), group_name);
  ARCANE_ASSERT((!mesh->findGroup(group_name).null()), ("Le groupe my_node_group n'a pas été créé"));
  PartialVariableNodeInt32 partial_node_var({ mesh, "partial_node_variable", mesh->nodeFamily()->name(), group_name });
  partial_node_var.fill(1);
  _checkVariable(partial_node_var, partial_node_var.itemGroup());
  // Groupe de face
  group_name = "my_face_group";
  _buildGroup(mesh->faceFamily(), group_name);
  ARCANE_ASSERT((!mesh->findGroup(group_name).null()), ("Le groupe my_face_group n'a pas été créé"));
  PartialVariableFaceInt32 partial_face_var({ mesh, "partial_face_variable", mesh->faceFamily()->name(), group_name });
  partial_face_var.fill(1);
  _checkVariable(partial_face_var, partial_face_var.itemGroup());
  // Groupe d'arête
  group_name = "my_edge_group";
  _buildGroup(mesh->edgeFamily(), group_name);
  ARCANE_ASSERT((!mesh->findGroup(group_name).null()), ("Le groupe my_edge_group n'a pas été créé"));
  PartialVariableEdgeInt32 partial_edge_var({ mesh, "partial_edge_variable", mesh->edgeFamily()->name(), group_name });
  partial_edge_var.fill(1);
  _checkVariable(partial_edge_var, partial_edge_var.itemGroup());

  for (const auto& group_infos : options()->checkGroup()) {
    auto group = mesh->findGroup(group_infos->getName());
    if (group.null())
      ARCANE_FATAL("Impossible de trouver le groupe {0}", group_infos->getName());
    ValueChecker vc{ A_FUNCINFO };
    auto group_size = 0;
    if (parallelMng()->isParallel()) {
      group_size = parallelMng()->reduce(Parallel::ReduceSum, group.own().size());
    }
    else {
      group_size = group.size();
    }
    vc.areEqual(group_size, group_infos->getSize(), "vérification de la taille du groupe");
  }
  ValueChecker vc{ A_FUNCINFO };
  auto nb_internal_group = 19;
  if (subDomain()->parallelMng()->isParallel()) {
    nb_internal_group = 27;
  }
  if (options()->nbMeshGroup.isPresent())
  {
    auto nb_group = nb_internal_group + options()->nbMeshGroup[0];
    vc.areEqual(nb_group, mesh->groups().count(), "vérification du nombre de groupes dans le maillage");
  }

  for (const auto& boundary_face_group_name : options()->getCheckBoundaryFaceGroup()) {
    _checkBoundaryFaceGroup(mesh, boundary_face_group_name);
  }
  for (const auto& boundary_face_group_name : options()->getCheckInternalFaceGroup()) {
    _checkInternalFaceGroup(mesh, boundary_face_group_name);
  }

  _checkBoundaryFaceGroup(mesh, mesh->outerFaces().name());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void MeshPolyhedralTestModule::
_testDimensions(IMesh* mesh)
{
  auto mesh_size = options()->meshSize();
  if (mesh_size.empty())
    return;
  auto nb_cell = mesh->nbCell();
  auto nb_face = mesh->nbFace();
  auto nb_edge = mesh->nbEdge();
  auto nb_node = mesh->nbNode();
  if (parallelMng()->isParallel()) {
    nb_cell = parallelMng()->reduce(Parallel::ReduceSum, mesh->ownCells().size());
    nb_face = parallelMng()->reduce(Parallel::ReduceSum, mesh->ownFaces().size());
    nb_edge = parallelMng()->reduce(Parallel::ReduceSum, mesh->ownEdges().size());
    nb_node = parallelMng()->reduce(Parallel::ReduceSum, mesh->ownNodes().size());
  }
  ValueChecker vc(A_FUNCINFO);
  vc.areEqual(nb_cell, mesh_size[0]->getNbCells(), "vérification du nombre de mailles");
  vc.areEqual(nb_face, mesh_size[0]->getNbFaces(), "vérification du nombre de faces");
  vc.areEqual(nb_edge, mesh_size[0]->getNbEdges(), "vérification du nombre d'arêtes");
  vc.areEqual(nb_node, mesh_size[0]->getNbNodes(), "vérification du nombre de nœuds");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void MeshPolyhedralTestModule::
_testCoordinates(Arcane::IMesh* mesh)
{
  if (options()->meshCoordinates.size() == 1) {
    if (options()->meshCoordinates[0].doCheck()) {
      auto node_coords = mesh->toPrimaryMesh()->nodesCoordinates();
      auto node_coords_ref = options()->meshCoordinates[0].coords();
      ValueChecker vc{ A_FUNCINFO };
      ENUMERATE_NODE (inode, allNodes()) {
        vc.areEqual(node_coords[inode], node_coords_ref[0]->value[inode.index()], "vérification des valeurs des coordonnées");
        debug(Trace::High) << " coordonnées des nœuds  " << node_coords[inode];
      }
    }
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void MeshPolyhedralTestModule::
_testMeshUtilities(Arcane::IMesh* mesh)
{
  auto* mesh_utilities = mesh->utilities();
  ARCANE_CHECK_POINTER(mesh_utilities);
  // Tester le changement de propriétaire à partir des mailles
  // test en séquentiel : changer virtuellement tous les propriétaires de mailles à 1 et vérifier que tous les éléments ont 1 comme propriétaire
  auto& cell_owners = mesh->cellFamily()->itemsNewOwner();
  auto& face_owners = mesh->faceFamily()->itemsNewOwner();
  auto& edge_owners = mesh->edgeFamily()->itemsNewOwner();
  auto& node_owners = mesh->nodeFamily()->itemsNewOwner();
  ENUMERATE_ (Cell, icell, allCells()) {
    cell_owners[icell] = 1;
    info() << "UID de la maille " << icell->uniqueId() << " cell_owner[icell] " << cell_owners[icell];
    info() << "La maille UID " << icell->uniqueId() << " a le propriétaire " << icell->owner();
  }
  mesh_utilities->changeOwnersFromCells();
  bool has_error = false;
  ENUMERATE_ (Face, iface, allFaces()) {
    has_error |= face_owners[iface] != 1;
  }
  ENUMERATE_ (Node, inode, allNodes()) {
    has_error |= node_owners[inode] != 1;
  }
  ENUMERATE_ (Edge, iedge, allEdges()) {
    has_error |= edge_owners[iedge] != 1;
  }
  if (has_error) {
    ARCANE_FATAL("changeOwnerFromCells ne fonctionne pas avec PolyhedralMesh");
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void MeshPolyhedralTestModule::
_testMeshModifier(Arcane::IMesh* mesh)
{
  auto* mesh_modifier = mesh->modifier();
  ARCANE_CHECK_POINTER(mesh_modifier);
  // Les méthodes suivantes ne sont pas encore implémentées. Ne rien faire en séquentiel et planter en parallèle
  mesh_modifier->addExtraGhostCellsBuilder(nullptr);
  mesh_modifier->removeExtraGhostCellsBuilder(nullptr);
  mesh_modifier->endUpdate(false, false);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void MeshPolyhedralTestModule::
_testConnectivity(IMesh* mesh)
{
  info() << "Test de connectivité";
  Connectivity connectivity{ mesh->connectivity() };
  ARCANE_FATAL_IF(!connectivity.hasConnectivity(Connectivity::eConnectivityType::CT_Default), "PolyhedralMesh doit avoir une connectivité standard");
  ARCANE_FATAL_IF(!connectivity.hasConnectivity(Connectivity::eConnectivityType::CT_EdgeConnectivity), "PolyhedralMesh doit avoir une connectivité d'arête");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void MeshPolyhedralTestModule::
_buildGroup(IItemFamily* family, String const& group_name)
{
  auto group = family->findGroup(group_name, true);
  Int32UniqueArray item_lids;
  item_lids.reserve(family->nbItem());
  ENUMERATE_ITEM (iitem, family->allItems()) {
    if (iitem.localId() % 2 == 0)
      item_lids.add(iitem.localId());
  }
  group.addItems(item_lids);
  info() << "Taille du groupe " << itemKindName(family->itemKind()) << " : " << group.size();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

template <typename VariableRefType>
void MeshPolyhedralTestModule::
_checkVariable(VariableRefType variable, ItemGroup item_group)
{
  _checkVariableWithRefValue(variable, item_group, item_group.size());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

template <typename VariableRefType>
void MeshPolyhedralTestModule::
_checkVariableWithRefValue(VariableRefType variable, Arcane::ItemGroup item_group, const typename VariableRefType::DataType& ref_sum)
{
  typename VariableRefType::DataType variable_sum = 0.;
  using ItemType = typename VariableRefType::ItemType;
  ENUMERATE_ (ItemType, iitem, item_group) {
    debug(Trace::High) << variable.name() << " à l'élément " << iitem.localId() << " " << variable[iitem];
    variable_sum += variable[iitem];
  }
  if (variable_sum != ref_sum && !parallelMng()->isParallel()) {
    fatal() << "Erreur sur la variable " << variable.name();
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

template <typename VariableArrayRefType>
void MeshPolyhedralTestModule::
_checkArrayVariable(VariableArrayRefType variable_ref, ItemGroup item_group)
{
  auto variable_sum = 0.;
  using ItemType = typename VariableArrayRefType::ItemType;
  auto array_size = variable_ref.arraySize();
  ENUMERATE_ (ItemType, iitem, item_group) {
    for (auto value : variable_ref[iitem]) {
      variable_sum += value;
    }
    debug(Trace::High) << variable_ref.name() << " à l'élément " << iitem.localId() << variable_ref[iitem];
  }
  ValueChecker vc{ A_FUNCINFO };
  std::vector<int> ref_sum(array_size);
  std::iota(ref_sum.begin(), ref_sum.end(), 1.);
  vc.areEqual(variable_sum, item_group.size() * std::accumulate(ref_sum.begin(), ref_sum.end(), 0.), "vérification des valeurs de la variable en tableau");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void MeshPolyhedralTestModule::
_checkBoundaryFaceGroup(IMesh* mesh, const String& boundary_face_group_name) const
{
  auto boundary_face_group = mesh->findGroup(boundary_face_group_name);
  if (boundary_face_group.null())
    ARCANE_FATAL("Impossible de trouver le groupe de faces de frontière {0}", boundary_face_group_name);
  bool are_face_boundaries = true;
  ENUMERATE_FACE (iface, boundary_face_group) {
    are_face_boundaries = are_face_boundaries && iface->isSubDomainBoundary();
    if (!iface->isSubDomainBoundary()) {
      debug(Trace::High) << String::format("La face {0} avec les nœuds {1} n'est pas une frontière", iface->uniqueId(), iface->nodes());
    }
  }
  if (!are_face_boundaries)
    ARCANE_FATAL("Le groupe de faces de frontière {0} contient une ou plusieurs faces qui ne sont pas sur la frontière du sous-domaine", boundary_face_group_name);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void MeshPolyhedralTestModule::
_checkInternalFaceGroup(IMesh* mesh, const String& internal_face_group_name) const
{
  auto internal_face_group = mesh->findGroup(internal_face_group_name);
  if (internal_face_group.null())
    ARCANE_FATAL("Impossible de trouver le groupe de faces internes {0}", internal_face_group_name);
  bool are_face_internals = true;
  ENUMERATE_FACE (iface, internal_face_group) {
    are_face_internals = are_face_internals && !iface->isSubDomainBoundary();
    if (iface->isSubDomainBoundary()) {
      debug(Trace::High) << String::format("La face {0} avec les nœuds {1} n'est pas une face interne", iface->uniqueId(), iface->nodes());
    }
  }
  if (!are_face_internals)
    ARCANE_FATAL("Le groupe de faces internes {0} contient une ou plusieurs faces qui sont sur la frontière du sous-domaine", internal_face_group_name);
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void MeshPolyhedralTestModule::
_checkFlags(IMesh* mesh) const
{
  bool are_flags_ok = true;
  bool has_internal_faces = false;
  ENUMERATE_FACE (iface, mesh->allFaces()) {
    Face face{ *iface };
    if (face.backCell().null()) {
      are_flags_ok = are_flags_ok && face.isSubDomainBoundary();
      are_flags_ok = are_flags_ok && (face.itemBase().flags() & ItemFlags::II_Boundary);
      are_flags_ok = are_flags_ok && (face.itemBase().flags() & ItemFlags::II_FrontCellIsFirst);
      are_flags_ok = are_flags_ok && !(face.itemBase().flags() & ItemFlags::II_BackCellIsFirst);
      are_flags_ok = are_flags_ok && (face.itemBase().flags() & ItemFlags::II_HasFrontCell);
      are_flags_ok = are_flags_ok && !(face.itemBase().flags() & ItemFlags::II_HasBackCell);
    }
    else if (face.frontCell().null()) {
      are_flags_ok = are_flags_ok && face.isSubDomainBoundary();
      are_flags_ok = are_flags_ok && (face.itemBase().flags() & ItemFlags::II_Boundary);
      are_flags_ok = are_flags_ok && (face.itemBase().flags() & ItemFlags::II_BackCellIsFirst);
      are_flags_ok = are_flags_ok && !(face.itemBase().flags() & ItemFlags::II_FrontCellIsFirst);
      are_flags_ok = are_flags_ok && (face.itemBase().flags() & ItemFlags::II_HasBackCell);
      are_flags_ok = are_flags_ok && !(face.itemBase().flags() & ItemFlags::II_HasFrontCell);
    }
    else {
      are_flags_ok = are_flags_ok && !face.isSubDomainBoundary();
      are_flags_ok = are_flags_ok && !(face.itemBase().flags() & ItemFlags::II_Boundary);
      are_flags_ok = are_flags_ok && (face.itemBase().flags() & ItemFlags::II_BackCellIsFirst);
      are_flags_ok = are_flags_ok && !(face.itemBase().flags() & ItemFlags::II_FrontCellIsFirst);
      are_flags_ok = are_flags_ok && (face.itemBase().flags() & ItemFlags::II_HasBackCell);
      are_flags_ok = are_flags_ok && (face.itemBase().flags() & ItemFlags::II_HasFrontCell);
      has_internal_faces = true;
    }
  }
  if (!are_flags_ok)
    ARCANE_FATAL("Les drapeaux de la face sont incorrects");
  if (has_internal_faces)
    info() << "Le maillage contient des faces internes";
  else
    info() << "Le maillage ne contient aucune face interne";
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

ARCANE_REGISTER_MODULE_MESHPOLYHEDRALTEST(MeshPolyhedralTestModule);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace ArcaneTest::MeshPolyhedral

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
