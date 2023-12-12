/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2020 Lawrence Livermore National Security LLC Copyright (c) 2018-2020 The Board of Trustees of the Leland Stanford
 *Junior University Copyright (c) 2018-2020 Total, S.A Copyright (c) 2019-     GEOSX Contributors All rights reserved
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
#include "mesh/MeshManager.hpp"
#include "mesh/generators/CellBlockManager.hpp"
#include "mesh/generators/VTKMeshGeneratorTools.hpp"
#include "RESQMLUtilities.hpp"
#include "mesh/mpiCommunications/CommunicationTools.hpp"


#include "EnergyMLDataObjectRepository.hpp"
#include "Region.hpp"
#include "Property.hpp"
#include "Surface.hpp"

#include <vtkBoundingBox.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataArray.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkExplicitStructuredGridToUnstructuredGrid.h>
#include <unordered_set>

#include <fesapi/resqml2/UnstructuredGridRepresentation.h>
#include <fesapi/resqml2/AbstractValuesProperty.h>
#include <fesapi/resqml2/SubRepresentation.h>



namespace geos
{

using namespace dataRepository;

RESQMLMeshGenerator::RESQMLMeshGenerator( string const & name,
                                          Group * const parent )
  : ExternalMeshGeneratorBase( name, parent ),
  m_repository( nullptr )
{
  registerWrapper( viewKeyStruct::repositoryString(), &m_objectName ).
    setInputFlag( InputFlags::REQUIRED ).
    setDescription( "Name of the EnergyML Repository" );

  registerWrapper( viewKeyStruct::uuidString(), &m_uuid ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "UUID of the mesh" );

  registerWrapper( viewKeyStruct::titleString(), &m_title ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "Title of the mesh in the EPC file" );

  registerWrapper( viewKeyStruct::regionAttributeString(), &m_attributeName ).
    setInputFlag( InputFlags::OPTIONAL ).
    setApplyDefaultValue( "attribute" ).
    setDescription( "Name of the VTK cell attribute to use as region marker" );

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

Group * RESQMLMeshGenerator::createChild( string const & childKey, string const & childName )
{
  GEOS_LOG_RANK_0( "Adding Mesh related entities: " << childKey << ", " << childName );
  if( childKey == groupKeyStruct::regionString() )
  {
    m_regions.emplace_back( childName );
    return &registerGroup< Region >( childName );
  }
  else if( childKey == groupKeyStruct::propertyString())
  {
    m_properties.emplace_back( childName );
    return &registerGroup< Property >( childName );
  }
  else if( childKey == groupKeyStruct::surfaceString())
  {
    m_surfaces.emplace_back(childName);
    return &registerGroup< Surface >( childName );
  }
  else
  {
    ExternalMeshGeneratorBase::createChild( childKey, childName );
  }
  return nullptr;
}

// void RESQMLMeshGenerator::expandObjectCatalogs()
// {
// During schema generation, register one of each type derived from EnergyMLDataObjectBase here
// for( auto & catalogIter: WellGeneratorBase::getCatalog())
// {
//   createChild( catalogIter.first, catalogIter.first );
// }

//  createChild( groupKeyStruct::regionString(), groupKeyStruct::regionString() );

// }

void RESQMLMeshGenerator::postProcessInput()
{
  MeshManager & meshManager = this->getGroupByPath< MeshManager >( "/Problem/Mesh" );
  // objectRepository.
  m_repository = meshManager.getGroupPointer< EnergyMLDataObjectRepository >( m_objectName );

  GEOS_THROW_IF( m_repository == nullptr,
                 getName() << ": EnergyML Data Object Repository not found: " << m_objectName,
                 InputError );

}

void RESQMLMeshGenerator::fillCellBlockManager( CellBlockManager & cellBlockManager, array1d< int > const & )
{
  GEOS_MARK_FUNCTION;

  MPI_Comm const comm = MPI_COMM_GEOSX;
  vtkSmartPointer< vtkMultiProcessController > controller = vtk::getController();
  vtkMultiProcessController::SetGlobalController( controller );

  GEOS_LOG_RANK_0( GEOS_FMT( "{} '{}': reading mesh", catalogName(), getName() ) );
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
  // m_repository = nullptr;
}


vtkSmartPointer< vtkDataSet >
RESQMLMeshGenerator::loadSurfaces( vtkSmartPointer< vtkDataSet > mesh )
{
  std::vector< std::pair<integer, RESQML2_NS::SubRepresentation *> > surfaces;

  for( const auto & s : m_surfaces )
  {
    Surface const & surface = this->getGroup< Surface >( s );

    GEOS_LOG(GEOS_FMT("{}", surface.getTitle()));

    if( !surface.getUUID().empty())
    {
      GEOS_LOG_RANK_0( GEOS_FMT( "{} '{}': reading surface {}", catalogName(), getName(), surface.getUUID() ) );

      auto * subrep = m_repository->getData()->getDataObjectByUuid< RESQML2_NS::SubRepresentation >( surface.getUUID() );
      if( subrep == nullptr )
        GEOS_ERROR( GEOS_FMT( "There exists no such data object with uuid {}", surface.getUUID() ) );

      if( subrep->getElementKindOfPatch( 0, 0 ) != gsoap_eml2_3::eml23__IndexableElement::faces)
        GEOS_ERROR( GEOS_FMT( "There subrepresentation {} must be a surface", surface.getUUID() ) );

      surfaces.push_back( std::make_pair( surface.getRegionId() , subrep) );
    }
    else if( !surface.getTitle().empty())
    {
      auto searchSurface = [&]( string const & surface_name ) -> RESQML2_NS::SubRepresentation *
      {
        for( auto * subrep : m_repository->getData()->getSubRepresentationSet())
        {
          if( subrep->getElementKindOfPatch( 0, 0 ) == gsoap_eml2_3::eml23__IndexableElement::faces  &&
              subrep->getTitle() == surface_name )
          {
            return subrep;
          }
        }

        return nullptr;
      };

      auto * subrep = searchSurface( surface.getTitle() );
      if( subrep == nullptr )
        GEOS_ERROR( GEOS_FMT( "There exists no such data object with title {}", surface.getTitle() ) );

      GEOS_LOG_RANK_0( GEOS_FMT( "{} '{}': reading surface {} - {}", catalogName(), getName(), subrep->getTitle(), subrep->getUuid() ) );
      surfaces.push_back( std::make_pair( surface.getRegionId(), subrep) );

    }
  }

  mesh = createSurfaces( mesh, surfaces, m_attributeName );

  return mesh;
}

vtkSmartPointer< vtkDataSet >
RESQMLMeshGenerator::loadRegions( vtkSmartPointer< vtkDataSet > mesh )
{
  std::vector< RESQML2_NS::SubRepresentation * > regions;

  for( const auto & r : m_regions )
  {
    Region const & region = this->getGroup< Region >( r );

    if( !region.getUUID().empty())
    {
      GEOS_LOG_RANK_0( GEOS_FMT( "{} '{}': reading region {}", catalogName(), getName(), region.getUUID() ) );

      auto * subrep = m_repository->getData()->getDataObjectByUuid< RESQML2_NS::SubRepresentation >( region.getUUID() );
      if( subrep == nullptr )
        GEOS_ERROR( GEOS_FMT( "There exists no such data object with uuid {}", region.getUUID() ) );

      if( subrep->getElementKindOfPatch( 0, 0 ) != gsoap_eml2_3::eml23__IndexableElement::cells)
        GEOS_ERROR( GEOS_FMT( "There subrepresentation {} must index cells elements in the mesh", region.getUUID() ) );

      regions.push_back( subrep );
    }
    else if( !region.getTitle().empty())
    {
      auto searchRegion = [&]( string const & region_name ) -> RESQML2_NS::SubRepresentation *
      {
        for( auto * subrep : m_repository->getData()->getSubRepresentationSet())
        {
          if( subrep->getElementKindOfPatch( 0, 0 ) == gsoap_eml2_3::eml23__IndexableElement::cells  &&
              subrep->getTitle() == region_name )
          {
            return subrep;
          }
        }

        return nullptr;
      };

      auto * subrep = searchRegion( region.getTitle() );
      if( subrep == nullptr )
        GEOS_ERROR( GEOS_FMT( "There exists no such data object with title {}", region.getTitle() ) );

      GEOS_LOG_RANK_0( GEOS_FMT( "{} '{}': reading region {} - {}", catalogName(), getName(), subrep->getTitle(), subrep->getUuid() ) );
      regions.push_back( subrep );

    }
  }

  mesh = createRegions( mesh, regions, m_attributeName );

  return mesh;
}


vtkSmartPointer< vtkDataSet >
RESQMLMeshGenerator::loadProperties( vtkSmartPointer< vtkDataSet > mesh )
{
  std::vector< RESQML2_NS::AbstractValuesProperty * > fields_list;

  for( const auto & p : m_properties )
  {

    Property const & property = this->getGroup< Property >( p );
    if( !property.getUUID().empty())
    {
      auto * prop = m_repository->getData()->getDataObjectByUuid< RESQML2_NS::AbstractValuesProperty >( property.getUUID() );
      if( prop == nullptr )
        GEOS_ERROR( GEOS_FMT( "There exists no such data object with uuid {} and title {}", property.getUUID() ) );

      GEOS_LOG_RANK_0( GEOS_FMT( "{} '{}': reading property {} - {}", catalogName(), getName(), prop->getTitle(), prop->getUuid() ) );

      fields_list.push_back( prop );

    }
    else if( !property.getTitle().empty())
    {
      auto searchPropery = [&]( string const & field_name ) -> RESQML2_NS::AbstractValuesProperty *
      {
        for( auto * value_prop : m_repository->getData()->getDataObjects< RESQML2_NS::AbstractValuesProperty >())
        {
          if( value_prop->getTitle() == field_name )
          {
            return value_prop;
          }
        }

        return nullptr;
      };

      auto * prop = searchPropery( property.getTitle() );
      if( prop == nullptr )
        GEOS_ERROR( GEOS_FMT( "There exists no such data object with title {}", property.getTitle() ) );

      GEOS_LOG_RANK_0( GEOS_FMT( "{} '{}': reading property {} - {}", catalogName(), getName(), prop->getTitle(), prop->getUuid() ) );

      fields_list.push_back( prop );

    }
  }

  // load the properties as fields
  for( unsigned int i = 0; i < fields_list.size(); ++i )
  {
    mesh = loadProperty( mesh, fields_list[i], m_properties[i] );
  }

  return mesh;
}


vtkSmartPointer< vtkDataSet >
RESQMLMeshGenerator::retrieveUnstructuredGrid()
{
  COMMON_NS::AbstractObject * rep{nullptr};

  if( !m_uuid.empty())
    rep = m_repository->getDataObject( m_uuid );
  else if( !m_title.empty())
    rep = m_repository->getDataObjectByTitle( m_title );

  if( rep == nullptr )
    GEOS_ERROR( GEOS_FMT( "There exists no such data object with uuid {} or title {} in the epc file", m_uuid, m_title ) );


  m_title = rep->getTitle();
  m_uuid = rep->getUuid();
  vtkSmartPointer< vtkDataSet > loadedMesh = loadGridRepresentation( rep );

  GEOS_LOG_RANK_0( GEOS_FMT( "GetNumberOfCells  {}", loadedMesh->GetNumberOfCells()) );
  GEOS_LOG_RANK_0( GEOS_FMT( "GetNumberOfPoints {}", loadedMesh->GetNumberOfPoints()) );

  return loadedMesh;
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


    if( loadedMesh->IsA( "vtkExplicitStructuredGrid" ))
    {
      vtkNew< vtkExplicitStructuredGridToUnstructuredGrid > ugConvertor;
      ugConvertor->SetInputData( loadedMesh );
      ugConvertor->Update();

      vtkNew< vtkXMLUnstructuredGridWriter > writer;
      writer->SetFileName( "tmp_output.vtu" );
      writer->SetInputData( ugConvertor->GetOutput() );
      writer->Write();
    }
    else
    {
      vtkNew< vtkXMLUnstructuredGridWriter > writer;
      writer->SetFileName( "tmp_output.vtu" );
      writer->SetInputData( loadedMesh );
      writer->Write();
    }

    return loadedMesh;
  }
  else
  {
    return vtkSmartPointer< vtkUnstructuredGrid >::New();
  }
}

std::tuple< string, string > RESQMLMeshGenerator::getParentRepresentation() const
{
  return {m_uuid, m_title};
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

REGISTER_CATALOG_ENTRY( MeshBase, RESQMLMeshGenerator, string const &, Group * const )

} // namespace geos
