/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2020 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2020 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2020 Total, S.A
 * Copyright (c) 2019-     GEOSX Contributors
 * All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

/**
 * @file RESQMLMeshGenerator.cpp
 */

#include "RESQMLMeshGenerator.hpp"


#include "common/DataTypes.hpp"
#include "common/DataLayouts.hpp"
#include "common/MpiWrapper.hpp"
#include "common/TypeDispatch.hpp"
#include "mesh/MeshBody.hpp"
#include "mesh/generators/CellBlockManager.hpp"
#include "mesh/generators/VTKMeshGeneratorTools.hpp"
#include "RESQMLUtilities.hpp"
#include "mesh/mpiCommunications/CommunicationTools.hpp"

#include <vtkBoundingBox.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataArray.h>

#include <unordered_set>

#include <fesapi/resqml2/UnstructuredGridRepresentation.h>
#include <fesapi/resqml2/AbstractValuesProperty.h>
#include <fesapi/resqml2/SubRepresentation.h>

#include <vtkUnstructuredGridWriter.h>

namespace geos
{

using namespace dataRepository;

RESQMLMeshGenerator::RESQMLMeshGenerator( string const & name,
                                          Group * const parent )
  : ExternalMeshGeneratorBase( name, parent ),
  m_repository( new common::DataObjectRepository())
{
  registerWrapper( viewKeyStruct::uuidString(), &m_parent_uuid ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "UUID of the mesh" );

  registerWrapper( viewKeyStruct::titleInFileString(), &m_parent_title ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "Title of the mesh in the EPC file" );

  registerWrapper( viewKeyStruct::uuidsRegionsToImportString(), &m_uuidsRegionsToImport ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "UUIDs of the RESQML subrepresentations to use as region marker" );

  registerWrapper( viewKeyStruct::regionAttributeString(), &m_attributeName ).
    setInputFlag( InputFlags::OPTIONAL ).
    setApplyDefaultValue( "attribute" ).
    setDescription( "Name of the VTK cell attribute to use as region marker" );

  registerWrapper( viewKeyStruct::uuidsFieldsToImportString(), &m_uuidsFieldsToImport ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "UUIDs of the RESQML properties to load as fields" );

  registerWrapper( viewKeyStruct::fieldsToImportString(), &m_fieldsToImport ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "Names of the RESQML properties to load as fields" );

  registerWrapper( viewKeyStruct::uuidsSurfacesToImportString(), &m_uuidsSurfacesToImport ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "UUIDs of the RESQML subrepresentations to use as surfaces" );

  registerWrapper( viewKeyStruct::partitionRefinementString(), &m_partitionRefinement ).
    setInputFlag( InputFlags::OPTIONAL ).
    setApplyDefaultValue( 1 ).
    setDescription( "Number of partitioning refinement iterations (defaults to 1, recommended value)."
                    "A value of 0 disables graph partitioning and keeps simple kd-tree partitions (not recommended). "
                    "Values higher than 1 may lead to slightly improved partitioning, but yield diminishing returns." );

  registerWrapper( viewKeyStruct::partitionMethodString(), &m_partitionMethod ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "Method (library) used to partition the mesh" );

  registerWrapper( viewKeyStruct::useGlobalIdsString(), &m_useGlobalIds ).
    setInputFlag( InputFlags::OPTIONAL ).
    setApplyDefaultValue( 0 ).
    setDescription( "Controls the use of global IDs in the input file for cells and points."
                    " If set to 0 (default value), the GlobalId arrays in the input mesh are used if available, and generated otherwise."
                    " If set to a negative value, the GlobalId arrays in the input mesh are not used, and generated global Ids are automatically generated."
                    " If set to a positive value, the GlobalId arrays in the input mesh are used and required, and the simulation aborts if they are not available" );

}

void RESQMLMeshGenerator::postProcessInput()
{
  GEOS_LOG_RANK_0( GEOS_FMT( "Reading: {}", m_filePath ) );
  COMMON_NS::EpcDocument pck( m_filePath );
  std::string message = pck.deserializeInto( *m_repository );
  pck.close();

  GEOS_LOG_RANK_0( GEOS_FMT( "Deserilization messages:\n {}", message ) );
}

void RESQMLMeshGenerator::fillCellBlockManager( CellBlockManager & cellBlockManager, array1d< int > const & )
{
  GEOS_MARK_FUNCTION;

  MPI_Comm const comm = MPI_COMM_GEOSX;
  vtkSmartPointer< vtkMultiProcessController > controller = vtk::getController();
  vtkMultiProcessController::SetGlobalController( controller );

  GEOS_LOG_RANK_0( GEOS_FMT( "{} '{}': reading mesh from {}", catalogName(), getName(), m_filePath ) );
  {
    GEOS_LOG_LEVEL_RANK_0( 2, "  reading the dataset..." );
    vtkSmartPointer< vtkDataSet > loadedMesh = loadMesh( );
    GEOS_LOG_LEVEL_RANK_0( 2, "  redistributing mesh..." );
    m_vtkMesh = vtk::redistributeMesh( loadedMesh, comm, m_partitionMethod, m_partitionRefinement, m_useGlobalIds );
    GEOS_LOG_LEVEL_RANK_0( 2, "  finding neighbor ranks..." );
    std::vector< vtkBoundingBox > boxes = vtk::exchangeBoundingBoxes( *m_vtkMesh, comm );
    std::vector< int > const neighbors = vtk::findNeighborRanks( std::move( boxes ) );
    m_spatialPartition.setMetisNeighborList( std::move( neighbors ));
    GEOS_LOG_LEVEL_RANK_0( 2, "  done!" );
  }

  GEOS_LOG_RANK_0( GEOS_FMT( "{} '{}': generating GEOS mesh data structure", catalogName(), getName() ) );


  GEOS_LOG_LEVEL_RANK_0( 2, "  preprocessing..." );
  m_cellMap = vtk::buildCellMap( *m_vtkMesh, m_attributeName );

  GEOS_LOG_LEVEL_RANK_0( 2, "  writing nodes..." );
  cellBlockManager.setGlobalLength( writeNodes( getLogLevel(), *m_vtkMesh, m_nodesetNames, cellBlockManager, this->m_translate, this->m_scale ) );

  GEOS_LOG_LEVEL_RANK_0( 2, "  writing cells..." );
  writeCells( getLogLevel(), *m_vtkMesh, m_cellMap, cellBlockManager );

  GEOS_LOG_LEVEL_RANK_0( 2, "  writing surfaces..." );
  writeSurfaces( getLogLevel(), *m_vtkMesh, m_cellMap, cellBlockManager );

  GEOS_LOG_LEVEL_RANK_0( 2, "  building connectivity maps..." );
  cellBlockManager.buildMaps();

  GEOS_LOG_LEVEL_RANK_0( 2, "  done!" );
  vtk::printMeshStatistics( *m_vtkMesh, m_cellMap, comm );
}

void RESQMLMeshGenerator::importFieldOnArray( Block block,
                                           string const & blockName,
                                           string const & meshFieldName,
                                           bool isMaterialField,
                                           dataRepository::WrapperBase & wrapper ) const
{
  GEOS_ASSERT_MSG( m_vtkMesh, "Must call generateMesh() before importFields()" );

  switch( block )
  {
    case MeshGeneratorBase::Block::VOLUMIC:
      return importVolumicFieldOnArray( blockName, meshFieldName, isMaterialField, wrapper );
    case MeshGeneratorBase::Block::SURFACIC:
    case MeshGeneratorBase::Block::LINEIC:
      return;
      // return importSurfacicFieldOnArray( blockName, meshFieldName, wrapper );
  }
}

void RESQMLMeshGenerator::freeResources()
{
  m_vtkMesh = nullptr;
  m_cellMap.clear();
  m_repository = nullptr;
}


vtkSmartPointer< vtkDataSet >
RESQMLMeshGenerator::loadSurfaces( vtkSmartPointer< vtkDataSet > mesh )
{
  //
  std::vector< resqml2::SubRepresentation * > surfaces;

  if( !m_uuidsSurfacesToImport.empty())
  {
    for( auto const & uuid : m_uuidsSurfacesToImport )
    {
      GEOS_LOG_RANK_0( GEOS_FMT( "{} '{}': reading surface {}", catalogName(), getName(), uuid ) );

      auto * subrep = m_repository->getDataObjectByUuid< resqml2::SubRepresentation >( uuid );
      if( subrep == nullptr )
        GEOS_ERROR( GEOS_FMT( "There exists no such data object with uuid {}", uuid ) );

      if( subrep->getElementKindOfPatch( 0, 0 ) != gsoap_eml2_3::eml23__IndexableElement::faces )
        GEOS_ERROR( GEOS_FMT( "The subrep with UUID {} is not a surface", uuid ) );

      surfaces.push_back( subrep );
    }
  }
  else
  {
    auto subrepSet = m_repository->getSubRepresentationSet();
    for( auto *subrep : subrepSet )
    {
      if( subrep->getElementKindOfPatch( 0, 0 ) == gsoap_eml2_3::eml23__IndexableElement::faces )
      {
        GEOS_LOG_RANK_0( GEOS_FMT( "{} '{}': reading surface {}", catalogName(), getName(), subrep->getUuid() ) );
        surfaces.push_back( subrep );
      }
    }
  }

  auto * meshWithSurfaces = createSurfaces( mesh, surfaces, m_attributeName );

  return vtkDataSet::SafeDownCast( meshWithSurfaces );
}

vtkSmartPointer< vtkDataSet >
RESQMLMeshGenerator::loadRegions( vtkSmartPointer< vtkDataSet > mesh )
{
  std::vector< resqml2::SubRepresentation * > regions;


  if( !m_uuidsRegionsToImport.empty())
  {
    for( auto const & region_uuid : m_uuidsRegionsToImport )
    {
      GEOS_LOG_RANK_0( GEOS_FMT( "{} '{}': reading region {}", catalogName(), getName(), region_uuid ) );

      auto * subrep = m_repository->getDataObjectByUuid< resqml2::SubRepresentation >( region_uuid );
      if( subrep == nullptr )
        GEOS_ERROR( GEOS_FMT( "There exists no such data object with uuid {}", region_uuid ) );

      regions.push_back( subrep );
    }
  }
  else
  {
    auto subrepSet = m_repository->getSubRepresentationSet();
    for( auto *subrep : subrepSet )
    {
      if( subrep->getElementKindOfPatch( 0, 0 ) == gsoap_eml2_3::eml23__IndexableElement::cells )
      {
        GEOS_LOG_RANK_0( GEOS_FMT( "{} '{}': reading region {}", catalogName(), getName(), subrep->getUuid() ) );
        regions.push_back( subrep );
      }
    }
  }

  mesh = createRegions( mesh, regions, m_attributeName );

  return mesh;
}


vtkSmartPointer< vtkDataSet >
RESQMLMeshGenerator::loadProperties( vtkSmartPointer< vtkDataSet > mesh )
{
  std::vector< RESQML2_NS::AbstractValuesProperty * > fields_list;

  if( !m_uuidsFieldsToImport.empty())
  {
    for( auto const & field_uuid : m_uuidsFieldsToImport )
    {
      auto * prop = m_repository->getDataObjectByUuid< RESQML2_NS::AbstractValuesProperty >( field_uuid );
      if( prop == nullptr )
        GEOS_ERROR( GEOS_FMT( "There exists no such data object with uuid {} and title {}", field_uuid ) );

      GEOS_LOG_RANK_0( GEOS_FMT( "{} '{}': reading property {} - {}", catalogName(), getName(), field_uuid, prop->getTitle() ) );

      fields_list.push_back( prop );
    }
  }
  else if( !this->m_fieldsToImport.empty())
  {
    auto searchPropery = [&]( string const & field_name ) -> RESQML2_NS::AbstractValuesProperty *
    {
      for( auto * value_prop : m_repository->getDataObjects< RESQML2_NS::AbstractValuesProperty >())
      {
        if( value_prop->getTitle() == field_name )
        {
          return value_prop;
        }
      }

      return nullptr;
    };

    for( auto const & field_name : m_fieldsToImport )
    {
      auto * prop = searchPropery( field_name );
      if( prop == nullptr )
        GEOS_ERROR( GEOS_FMT( "There exists no such data object with uuid {} and title {}", field_name ) );

      GEOS_LOG_RANK_0( GEOS_FMT( "{} '{}': reading property {} - {}", catalogName(), getName(), field_name, prop->getUuid() ) );

      fields_list.push_back( prop );
    }
  }

  // load the properties as fields
  for( unsigned int i = 0; i < fields_list.size(); ++i )
  {
    mesh = loadProperty( mesh, fields_list[i], this->m_volumicFieldsToImport[i] );
  }

  return mesh;
}


vtkSmartPointer< vtkDataSet >
RESQMLMeshGenerator::retrieveUnstructuredGrid()
{
  RESQML2_NS::UnstructuredGridRepresentation * rep{nullptr};

  if( !m_parent_uuid.empty())
  {
    rep = m_repository->getDataObjectByUuid< RESQML2_NS::UnstructuredGridRepresentation >( m_parent_uuid );

  }
  else if( !m_parent_title.empty())
  {
    auto grid_set = m_repository->getUnstructuredGridRepresentationSet();
    auto result = std::find_if( std::begin( grid_set ), std::end( grid_set ), [&m_parent_title = m_parent_title]( RESQML2_NS::UnstructuredGridRepresentation *grid ) {
      return grid->getTitle() == m_parent_title;
    } );
    if( result != std::end( grid_set ))
    {
      rep = *result;
    }
  }
  else
  {
    rep = m_repository->getUnstructuredGridRepresentationSet().front();
  }

  if( rep == nullptr )
    GEOS_ERROR( GEOS_FMT( "There exists no such data object with uuid {} or title {} in the epc file", m_parent_uuid, m_parent_title ) );


  m_parent_title = rep->getTitle();
  m_parent_uuid = rep->getUuid();
  vtkSmartPointer< vtkUnstructuredGrid > loadedMesh = loadUnstructuredGridRepresentation( rep );

  GEOS_LOG_RANK_0( GEOS_FMT( "GetNumberOfCells  {}", loadedMesh->GetNumberOfCells()) );
  GEOS_LOG_RANK_0( GEOS_FMT( "GetNumberOfPoints {}", loadedMesh->GetNumberOfPoints()) );

  return vtkDataSet::SafeDownCast( loadedMesh );
}

vtkSmartPointer< vtkDataSet >
RESQMLMeshGenerator::loadMesh()
{
  if( MpiWrapper::commRank() == 0 )
  {
    GEOS_LOG_LEVEL_RANK_0( 2, "  reading the RESQML dataset..." );
    vtkSmartPointer< vtkDataSet > loadedMesh = retrieveUnstructuredGrid( );

    GEOS_LOG_LEVEL_RANK_0( 2, "  (regions) load the RESQML subrepresentations into vtk attributes..." );
    loadedMesh = loadRegions( loadedMesh );

    GEOS_LOG_LEVEL_RANK_0( 2, "  (fields) load the RESQML Properties into vtk attributes..." );
    loadedMesh = loadProperties( loadedMesh );

    GEOS_LOG_LEVEL_RANK_0( 2, "  (surfaces) load the RESQML subrepresentations into the vtk grid..." );
    loadedMesh = loadSurfaces( loadedMesh );

    GEOS_LOG_LEVEL_RANK_0( 2, "  ... end" );

    GEOS_LOG_LEVEL_RANK_0( 2, "  temporary write" );
    vtkNew< vtkUnstructuredGridWriter > writer;
    writer->SetFileName( "tmp_output.vtu" );
    writer->SetInputData( loadedMesh );
    writer->Write();

    return loadedMesh;
  }
  else
  {
    return vtkSmartPointer< vtkUnstructuredGrid >::New();
  }
}

std::tuple< string, string > RESQMLMeshGenerator::getParentRepresentation() const
{
  return {m_parent_uuid, m_parent_title};
}

void RESQMLMeshGenerator::importVolumicFieldOnArray( string const & cellBlockName,
                                                  string const & meshFieldName,
                                                  bool isMaterialField,
                                                  dataRepository::WrapperBase & wrapper ) const
{
  for( auto const & typeRegions : m_cellMap )
  {
    // Restrict data import to 3D cells
    if( getElementDim( typeRegions.first ) == 3 )
    {
      for( auto const & regionCells: typeRegions.second )
      {
        string const currentCellBlockName = vtk::buildCellBlockName( typeRegions.first, regionCells.first );
        // We don't know how the user mapped cell blocks to regions, so we must check all of them
        if( cellBlockName != currentCellBlockName )
          continue;

        vtkDataArray * vtkArray = vtk::findArrayForImport( *m_vtkMesh, meshFieldName );
        if( isMaterialField )
        {
          return vtk::importMaterialField( regionCells.second, vtkArray, wrapper );
        }
        else
        {
          return vtk::importRegularField( regionCells.second, vtkArray, wrapper );
        }
      }
    }
  }

  GEOS_ERROR( "Could not import field \"" << meshFieldName << "\" from cell block \"" << cellBlockName << "\"." );
}

REGISTER_CATALOG_ENTRY( MeshGeneratorBase, RESQMLMeshGenerator, string const &, Group * const )

} // namespace geos