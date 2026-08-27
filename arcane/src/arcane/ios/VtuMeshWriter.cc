// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* VtuMeshWriter.cc                                            (C) 2000-2026 */
/*                                                                           */
/* Lecture/Ecriture d'un fichier au format VtuMeshWriter.                    */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/utils/ArcanePrecomp.h"
#include "arcane/utils/Iostream.h"
#include "arcane/utils/StdHeader.h"
#include "arcane/utils/HashTableMap.h"
#include "arcane/utils/ValueConvert.h"
#include "arcane/utils/ScopedPtr.h"
#include "arcane/utils/ITraceMng.h"
#include "arcane/utils/String.h"
#include "arcane/utils/IOException.h"
#include "arcane/utils/Collection.h"
#include "arcane/utils/Enumerator.h"
#include "arcane/utils/NotImplementedException.h"
#include "arcane/utils/Real3.h"
#include "arcane/utils/PlatformUtils.h"

#include "arcane/core/FactoryService.h"
#include "arcane/core/IMainFactory.h"
#include "arcane/core/IMeshReader.h"
#include "arcane/core/ISubDomain.h"
#include "arcane/core/IMesh.h"
#include "arcane/core/IMeshSubMeshTransition.h"
#include "arcane/core/IItemFamily.h"
#include "arcane/core/Item.h"
#include "arcane/core/ItemEnumerator.h"
#include "arcane/core/VariableTypes.h"
#include "arcane/core/IVariableAccessor.h"
#include "arcane/core/IParallelMng.h"
#include "arcane/core/IIOMng.h"
#include "arcane/core/IXmlDocumentHolder.h"
#include "arcane/core/XmlNodeList.h"
#include "arcane/core/XmlNode.h"
#include "arcane/core/IMeshUtilities.h"
#include "arcane/core/IMeshWriter.h"
#include "arcane/core/BasicService.h"
#include "arcane/core/AbstractService.h"

#include <vtkXMLUnstructuredGridReader.h>
#include <vtkIdTypeArray.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCell.h>
#include <vtkObject.h>
#include <vtkDataObject.h>
#include <vtkDataSet.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkLongArray.h>
#include <vtkIntArray.h>
#include <vtkFieldData.h>

#include <map>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Écriture des fichiers de maillage au format VTU (de VTK).
 */
class VtuMeshWriter
: public AbstractService
, public IMeshWriter
{
 public:

  VtuMeshWriter(const ServiceBuildInfo& sbi)
  : AbstractService(sbi)
  {}

  virtual void build() {}
  virtual bool writeMeshToFile(IMesh* mesh, const String& file_name);

 protected:

  void _writeFieldGroupsFromData(vtkFieldData* field_data, ItemGroup groups);
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

ARCANE_REGISTER_SERVICE(VtuMeshWriter,
                        ServiceProperty("VtuNewMeshWriter", ST_SubDomain),
                        ARCANE_SERVICE_INTERFACE(IMeshWriter));

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*****************************************************************\
 * _writeFieldGroupsFromData
	uniqueId().asInteger
	localId
\*****************************************************************/
void VtuMeshWriter::
_writeFieldGroupsFromData(vtkFieldData* fieldData, ItemGroup group)
{
  vtkLongArray* a = vtkLongArray::New();
  a->SetName(group.name().localstr());

  a->InsertNextValue(group.itemKind());

  ENUMERATE_ITEM (iitem, group) {
    a->InsertNextValue(iitem->uniqueId().asInt32());
  }

  a->SetNumberOfTuples(a->GetNumberOfTuples());
  fieldData->AddArray(a);
  a->Delete();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
  \brief Écrit un maillage.
	Les données sont stockées selon ARCANE_VTU_DATA_MODE_TO_[ASCII|BINARY],
	le BINARY étant celui par défaut.

	\param mesh maillage à écrire
	\param file_name nom du fichier
	\retval true pour une erreur
  \retval false pour un succès	
*/
bool VtuMeshWriter::
writeMeshToFile(IMesh* mesh, const String& file_name)
{

  Integer mesh_nb_node = mesh->nbNode();
  Integer mesh_nb_cell = mesh->nbCell();
  std::map<Int64, Integer> uid_to_idx_map; // Tableau des IDs uniques des nœuds
  info() << "[VtuMeshWriter::writeMeshToFile] mesh_nb_node=" << mesh_nb_node << " mesh_nb_cell=" << mesh_nb_cell
         << " all=" << mesh->allNodes().size() << ", own=" << mesh->ownNodes().size();

  /*************************\
	* Initialisation côté VTK *
	\*************************/
  vtkPoints* points = vtkPoints::New();
  points->SetDataTypeToDouble();
  vtkUnstructuredGrid* grid = vtkUnstructuredGrid::New();
  grid->Allocate(mesh_nb_cell, mesh_nb_cell);

  /*************************\
	* Initialisation côté ARC *
	\*************************/
  VariableItemReal3& nodes_coords = PRIMARYMESH_CAST(mesh)->nodesCoordinates();

  /*************************\
	* Sauvegarde des IDs uniques des mailles *
	\*************************/
  info() << "[writeMeshToFile] Création du tableau des IDs uniques des MAILLES";
  vtkCellData* vtk_cell_data = grid->GetCellData(); // Données associées aux Mailles
  vtkLongArray* vtk_cell_uids = vtkLongArray::New();
  vtk_cell_uids->SetName("CellsUniqueIDs");
  ENUMERATE_CELL (iCell, mesh->allCells()) {
    Cell cell = *iCell;
    vtk_cell_uids->InsertNextValue(cell.uniqueId().asInt32());
  }
  vtk_cell_data->AddArray(vtk_cell_uids); // Ajout de notre tableau d'IDs uniques des Mailles
  vtk_cell_uids->Delete();

  /*************************\
	* Sauvegarde des IDs uniques des nœuds *
	\*************************/
  info() << "[writeMeshToFile] Création du tableau des IDs uniques des NŒUDS";
  vtkPointData* vtk_point_data = grid->GetPointData(); // Données associées aux Points
  vtkLongArray* vtk_point_uids = vtkLongArray::New();
  vtk_point_uids->SetName("NodesUniqueIDs");
  vtkIntArray* vtk_point_owners = vtkIntArray::New();
  vtk_point_owners->SetName("NodesOwner");
  Integer index = 0;
  ENUMERATE_NODE (inode, mesh->allNodes()) {
    Node node = *inode;
    Int64 uid = node.uniqueId();
    Real3 coord = nodes_coords[inode];
    points->InsertNextPoint(Convert::toDouble(coord.x),
                            Convert::toDouble(coord.y),
                            Convert::toDouble(coord.z));
    uid_to_idx_map.insert(std::make_pair(uid, index));
    vtk_point_uids->InsertNextValue(uid);
    vtk_point_owners->InsertNextValue(node.owner());
    ++index;
  }
  vtk_point_data->AddArray(vtk_point_uids);
  vtk_point_uids->Delete();

  vtk_point_data->AddArray(vtk_point_owners);
  vtk_point_owners->Delete();

  /*****************************\
	* Maintenant, définir les points dans le maillage *
	\*****************************/
  info() << "[writeMeshToFile] Maintenant, définir les points dans le maillage";
  grid->SetPoints(points);

  /**********************************************\
	* Balayage des nœuds des mailles pour créer la connectivité *
	\**********************************************/
  info() << "[writeMeshToFile] Maintenant, balayer les nœuds des mailles pour créer la connectivité";
  vtkIdList* vtk_point_ids = vtkIdList::New();
  ENUMERATE_CELL (iCell, mesh->allCells()) {
    Cell cell = *iCell;
    int nb_node = cell.nbNode();
    vtk_point_ids->Allocate(nb_node);

    for (Integer j = 0; j < nb_node; ++j) {
      Int64 node_uid = cell.node(j).uniqueId();
      auto x = uid_to_idx_map.find(node_uid);
      if (x == uid_to_idx_map.end())
        ARCANE_FATAL("InternalError: no index for uid '{0}'", node_uid);
      vtk_point_ids->InsertNextId(x->second);
    }

    int vtk_item = IT_NullType;
    switch (cell.type()) {
    // Mailles linéaires
    case (IT_Tetraedron4):
      vtk_item = VTK_TETRA;
      break;
    case (IT_Hexaedron8):
      vtk_item = VTK_HEXAHEDRON;
      break;
    case (IT_Pyramid5):
      vtk_item = VTK_PYRAMID;
      break;

    // Générateur de maillage simple-1&2
    case (IT_Octaedron12):
      vtk_item = VTK_HEXAGONAL_PRISM;
      break;
    case (IT_Heptaedron10):
      vtk_item = VTK_PENTAGONAL_PRISM;
      break;

      // Prisme
    case (IT_Pentaedron6):
      vtk_item = VTK_WEDGE;
      break;

    // À implémenter
    case (IT_HemiHexa7):
      info() << "VTK_POLY_VERTEX";
      vtk_item = VTK_POLY_VERTEX;
      break;
    case (IT_HemiHexa6):
      info() << "VTK_POLY_VERTEX";
      vtk_item = VTK_POLY_VERTEX;
      break;
    case (IT_HemiHexa5):
      info() << "VTK_POLY_VERTEX";
      vtk_item = VTK_POLY_VERTEX;
      break;
    case (IT_AntiWedgeLeft6):
      info() << "VTK_POLY_VERTEX";
      vtk_item = VTK_POLY_VERTEX;
      break;
    case (IT_AntiWedgeRight6):
      info() << "VTK_POLY_VERTEX";
      vtk_item = VTK_POLY_VERTEX;
      break;
    case (IT_DiTetra5):
      info() << "VTK_POLY_VERTEX";
      vtk_item = VTK_POLY_VERTEX;
      break;
      /* Autres non encore implémentés */
    default:
      info() << "[writeMeshToFile] Type de maille non pris en charge (" << cell.type() << ")";
      throw NotSupportedException(A_FUNCINFO);
    }
    grid->InsertNextCell(vtk_item, vtk_point_ids);
    vtk_point_ids->Reset();
  }
  vtk_point_ids->Delete();

  /***********************\
	* Récupération des autres groupes *
	\***********************/
  info() << "[writeMeshToFile] ## Maintenant Récupération des Groupes ##";
  vtkFieldData* vtkFieldDataGroups = grid->GetFieldData();

  for (ItemGroupCollection::Enumerator igroup(mesh->groups()); ++igroup;) {
    if (igroup->isAllItems())
      continue;
    info() << "[writeMeshToFile] Trouvé un groupe de type " << igroup->itemKind() << " " << igroup->name();
    _writeFieldGroupsFromData(vtkFieldDataGroups, *igroup);
  }

  /************************\
	* Préparation pour la sortie *
	\************************/
  vtkXMLUnstructuredGridWriter* vtk_grid_writer = vtkXMLUnstructuredGridWriter::New();
  vtk_grid_writer->SetInputData(grid);
  String fileNameDotVtu(file_name);
  if (!file_name.endsWith(".vtu"))
    fileNameDotVtu = file_name + ".vtu";
  vtk_grid_writer->SetFileName(fileNameDotVtu.localstr());
  info() << "[writeMeshToFile] SetFileName " << fileNameDotVtu;
  String isAscii = platform::getEnvironmentVariable("ARCANE_VTU_DATA_MODE_TO_ASCII");
  if (!isAscii.null())
    vtk_grid_writer->SetDataModeToAscii();
  else
    vtk_grid_writer->SetDataModeToBinary();
  vtk_grid_writer->Write();

  /**********************\
	* Et un petit nettoyage *
	\**********************/
  points->Delete();
  grid->Delete();
  vtk_grid_writer->Delete();
  info() << "[writeMeshToFile] Terminé";
  return false;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
