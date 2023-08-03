/**
 * @file RESQMLMeshGenerator.cpp
 */

#include "RESQMLMeshGenerator.hpp"


#include "common/DataTypes.hpp"
#include "common/DataLayouts.hpp"
#include "common/MpiWrapper.hpp"
#include "common/TypeDispatch.hpp"
#include "mesh/DomainPartition.hpp"
#include "mesh/MeshBody.hpp"
#include "mesh/generators/CellBlockManager.hpp"
#include "mesh/generators/VTKMeshGeneratorTools.hpp"
#include "mesh/mpiCommunications/CommunicationTools.hpp"

#include <vtkBoundingBox.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataArray.h>
#include <vtkAppendFilter.h>
#include <unordered_set>

#include "mesh/generators/RESQML/vtkRESQMLUnstructuredGridReader.h"
#include "mesh/generators/RESQML/vtkRESQMLSubRepresentationReader.h"
#include "mesh/generators/RESQML/vtkRESQMLPropertyReader.h"
#include "mesh/generators/RESQML/vtkCellSetsToCellData.h"
// #include "mesh/generators/RESQML/vtkAppendCellSetsFilter.h"

#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkGenerateGlobalIds.h>
namespace geosx
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
  GEOSX_ERROR_IF( m_parent_title.empty() && m_parent_uuid.empty(), "Either `titleInFile` or `UUID` must be present to load the RESQML Representation from the EPC file" );

  GEOSX_LOG_RANK_0( GEOSX_FMT( "Reading: {}", m_filePath ) );
  COMMON_NS::EpcDocument pck( m_filePath );
  std::string message = pck.deserializeInto( *m_repository );
  pck.close();

  GEOSX_LOG_RANK_0( GEOSX_FMT( "Deserilization messages:\n {}", message ) );
}

vtkSmartPointer< vtkDataSet >
RESQMLMeshGenerator::loadDataObject()
{
  auto * rep = retrieveUnstructuredGrid();

  vtkNew< vtkRESQMLUnstructuredGridReader > reader;

  reader->SetUnstructuredGridRepresentation( rep );
  reader->Update();
  vtkSmartPointer< vtkDataSet > loadedMesh = vtkDataSet::SafeDownCast( reader->GetOutput());

  GEOSX_LOG_RANK_0( GEOSX_FMT( "GetNumberOfCells  {}", loadedMesh->GetNumberOfCells()) );
  GEOSX_LOG_RANK_0( GEOSX_FMT( "GetNumberOfPoints {}", loadedMesh->GetNumberOfPoints()) );

  return loadedMesh;
}

vtkSmartPointer< vtkDataSet >
RESQMLMeshGenerator::loadRegions( vtkDataSet & mesh )
{
  vtkNew< vtkCellSetsToCellData > filter;
  filter->SetName( m_attributeName );
  filter->SetRepresentation( &mesh );
  for( int i = 0; i < m_uuidsRegionsToImport.size(); ++i )
  {
    auto const & region_uuid = m_uuidsRegionsToImport[i];

    GEOSX_LOG_RANK_0( GEOSX_FMT( "{} '{}': reading region {}", catalogName(), getName(), region_uuid ) );

    auto * subrep = m_repository->getDataObjectByUuid< resqml2::SubRepresentation >( region_uuid );
    if( subrep == nullptr )
      GEOSX_ERROR( GEOSX_FMT( "There exists no such data object with uuid {}", region_uuid ) );

    vtkNew< vtkRESQMLSubRepresentationReader > subrep_reader;
    subrep_reader->SetInputDataObject( &mesh );
    subrep_reader->SetSubRepresentation( subrep );
    subrep_reader->Update();

    GEOSX_LOG_RANK_0( GEOSX_FMT( "loadRegions {} {}", subrep->getTitle(), subrep_reader->GetOutput()->GetNumberOfCells()) );

    filter->AddInputData( subrep_reader->GetOutput());
  }
  filter->Update();

  return vtkDataSet::SafeDownCast( filter->GetOutputDataObject( 0 ));
}

vtkSmartPointer< vtkDataSet >
RESQMLMeshGenerator::loadFields( vtkSmartPointer< vtkDataSet > mesh )
{
  std::vector< RESQML2_NS::AbstractValuesProperty * > fields_list;

  if( !m_uuidsFieldsToImport.empty())
  {
    for( auto const & field_uuid : m_uuidsFieldsToImport )
    {
      auto * prop = m_repository->getDataObjectByUuid< RESQML2_NS::AbstractValuesProperty >( field_uuid );
      if( prop == nullptr )
        GEOSX_ERROR( GEOSX_FMT( "There exists no such data object with uuid {} and title {}", field_uuid ) );

      GEOSX_LOG_RANK_0( GEOSX_FMT( "{} '{}': reading property {} - {}", catalogName(), getName(), field_uuid, prop->getTitle() ) );

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
        GEOSX_ERROR( GEOSX_FMT( "There exists no such data object with uuid {} and title {}", field_name ) );

      GEOSX_LOG_RANK_0( GEOSX_FMT( "{} '{}': reading property {} - {}", catalogName(), getName(), field_name, prop->getUuid() ) );

      fields_list.push_back( prop );
    }
  }

  for( unsigned int i = 0; i < fields_list.size(); ++i )
  {
    vtkNew< vtkRESQMLPropertyReader > prop_reader;
    prop_reader->SetInputData( mesh );
    prop_reader->SetValuesProperty( fields_list[i] );
    prop_reader->SetName( m_fieldNamesInGEOSX[i] );
    prop_reader->Update();

    mesh = prop_reader->GetOutput();
  }

  return mesh;
}

vtkSmartPointer< vtkDataSet >
RESQMLMeshGenerator::loadSurfaces( vtkDataSet & mesh )
{
  // vtkNew<vtkAppendCellSetsFilter> filter;
  // filter->AddInputData(0, &mesh);
  vtkNew<vtkAppendFilter> filter;
  filter->AddInputData(vtkUnstructuredGrid::SafeDownCast(&mesh));
  for( int i = 0; i < m_uuidsSurfacesToImport.size(); ++i )
  {
    auto const & uuid = m_uuidsSurfacesToImport[i];

    GEOSX_LOG_RANK_0( GEOSX_FMT( "{} '{}': reading surface {}", catalogName(), getName(), uuid ) );

    auto * subrep = m_repository->getDataObjectByUuid< resqml2::SubRepresentation >( uuid );
    if( subrep == nullptr )
      GEOSX_ERROR( GEOSX_FMT( "There exists no such data object with uuid {}", uuid ) );

    vtkNew< vtkRESQMLSubRepresentationReader > subrep_reader;
    subrep_reader->SetInputDataObject( &mesh );
    subrep_reader->SetSubRepresentation( subrep );
    subrep_reader->Update();


    vtkNew<vtkXMLUnstructuredGridWriter> writer;
    writer->SetFileName("test.vtu");
    writer->SetInputData(subrep_reader->GetOutput());
    writer->Write();

    // filter->AddInputConnection(1, subrep_reader->GetOutputPort());
    filter->AddInputData(subrep_reader->GetOutput());

    GEOSX_LOG_RANK_0( GEOSX_FMT( "loadSurfaces {} {}", subrep->getTitle(), subrep_reader->GetOutput()->GetNumberOfCells()) );
  }
  filter->Update();

  // return &mesh;
  
  //return vtkDataSet::SafeDownCast( filter->GetOutputDataObject( 0 ));
    return vtkDataSet::SafeDownCast( filter->GetOutput());
}

resqml2::UnstructuredGridRepresentation *
RESQMLMeshGenerator::retrieveUnstructuredGrid()
{
  if( m_parent_uuid.empty())
  {
    for( auto * grid : m_repository->getUnstructuredGridRepresentationSet())
    {
      if( grid->getTitle() == m_parent_title )
      {
        m_parent_uuid = grid->getUuid();
        return grid;
      }
    }
    GEOSX_ERROR( GEOSX_FMT( "There exists no such data object with name {}", m_parent_title ) );
  }

  auto * rep = m_repository->getDataObjectByUuid< resqml2::UnstructuredGridRepresentation >( m_parent_uuid );

  if( rep == nullptr )
    GEOSX_ERROR( GEOSX_FMT( "There exists no such data object with uuid {}", m_parent_uuid ) );

  m_parent_title = rep->getTitle();

  return rep;
}

vtkSmartPointer< vtkDataSet >
RESQMLMeshGenerator::loadMesh()
{
  if( MpiWrapper::commRank() == 0 )
  {
    GEOSX_LOG_LEVEL_RANK_0( 2, "  reading the RESQML dataset..." );
    vtkSmartPointer< vtkDataSet > loadedMesh = loadDataObject( );

    if( !m_uuidsRegionsToImport.empty())
    {
      GEOSX_LOG_LEVEL_RANK_0( 2, "  (regions) load the RESQML subrepresentations into vtk attributes..." );
      loadRegions( *loadedMesh );
    }

    if( !m_uuidsFieldsToImport.empty() || !this->m_fieldsToImport.empty())
    {
      GEOSX_LOG_LEVEL_RANK_0( 2, "  (fields) load the RESQML Properties into vtk attributes..." );
      loadedMesh = loadFields( loadedMesh );
    }

    if( !m_uuidsSurfacesToImport.empty())
    {
      GEOSX_LOG_LEVEL_RANK_0( 2, "  (surfaces) load the RESQML subrepresentations into vtk unstructured grid..." );
      loadedMesh = loadSurfaces( *loadedMesh );
    }
    GEOSX_LOG_LEVEL_RANK_0( 2, "  ... end" );

    vtkNew<vtkXMLUnstructuredGridWriter> writer;
    writer->SetFileName("output.vtu");
    writer->SetInputData(loadedMesh);
    writer->Write();



    return loadedMesh;
  }
  else
  {
    return vtkSmartPointer< vtkUnstructuredGrid >::New();
  }
}

void RESQMLMeshGenerator::generateMesh( DomainPartition & domain )
{
  GEOSX_MARK_FUNCTION;

  MPI_Comm const comm = MPI_COMM_GEOSX;
  vtkSmartPointer< vtkMultiProcessController > controller = vtk::getController();
  vtkMultiProcessController::SetGlobalController( controller );

  GEOSX_LOG_RANK_0( GEOSX_FMT( "{} '{}': reading mesh from {}", catalogName(), getName(), m_filePath ) );
  {
    GEOSX_LOG_LEVEL_RANK_0( 2, "  reading the dataset..." );
    vtkSmartPointer< vtkDataSet > loadedMesh = loadMesh( );
    GEOSX_LOG_LEVEL_RANK_0( 2, "  redistributing mesh..." );
    m_vtkMesh = vtk::redistributeMesh( *loadedMesh, comm, m_partitionMethod, m_partitionRefinement, m_useGlobalIds );
    GEOSX_LOG_LEVEL_RANK_0( 2, "  finding neighbor ranks..." );
    std::vector< vtkBoundingBox > boxes = vtk::exchangeBoundingBoxes( *m_vtkMesh, comm );
    std::vector< int > const neighbors = vtk::findNeighborRanks( std::move( boxes ) );
    domain.getMetisNeighborList().insert( neighbors.begin(), neighbors.end() );
    GEOSX_LOG_LEVEL_RANK_0( 2, "  done!" );
  }

  GEOSX_LOG_RANK_0( GEOSX_FMT( "{} '{}': generating GEOSX mesh data structure", catalogName(), getName() ) );

  MeshBody & meshBody = domain.getMeshBodies().registerGroup< MeshBody >( this->getName() );
  meshBody.createMeshLevel( 0 );

  CellBlockManager & cellBlockManager = meshBody.registerGroup< CellBlockManager >( keys::cellManager );

  GEOSX_LOG_LEVEL_RANK_0( 2, "  preprocessing..." );
  m_cellMap = vtk::buildCellMap( *m_vtkMesh, m_attributeName );

  GEOSX_LOG_LEVEL_RANK_0( 2, "  writing nodes..." );
  real64 const globalLength = writeNodes( getLogLevel(), *m_vtkMesh, m_nodesetNames, cellBlockManager, this->m_translate, this->m_scale );
  meshBody.setGlobalLengthScale( globalLength );

  GEOSX_LOG_LEVEL_RANK_0( 2, "  writing cells..." );
  writeCells( getLogLevel(), *m_vtkMesh, m_cellMap, cellBlockManager );

  GEOSX_LOG_LEVEL_RANK_0( 2, "  writing surfaces..." );
  writeSurfaces( getLogLevel(), *m_vtkMesh, m_cellMap, cellBlockManager );

  GEOSX_LOG_LEVEL_RANK_0( 2, "  building connectivity maps..." );
  cellBlockManager.buildMaps();

  GEOSX_LOG_LEVEL_RANK_0( 2, "  done!" );
  vtk::printMeshStatistics( *m_vtkMesh, m_cellMap, comm );
}

void RESQMLMeshGenerator::importFields( DomainPartition & domain ) const
{
  GEOSX_LOG_RANK_0( GEOSX_FMT( "{} '{}': importing field data from mesh dataset", catalogName(), getName() ) );
  GEOSX_ASSERT_MSG( m_vtkMesh, "Must call generateMesh() before importFields()" );

  // TODO Having CellElementSubRegion and ConstitutiveBase... here in a pure geometric module is problematic.
  ElementRegionManager & elemManager = domain.getMeshBody( this->getName() ).getBaseDiscretization().getElemManager();

  std::vector< vtkDataArray * > const srcArrays = vtk::findArraysForImport( *m_vtkMesh, m_fieldNamesInGEOSX );

  FieldIdentifiers fieldsToBeSync;

  for( auto const & typeRegions : m_cellMap )
  {
    // Restrict data import to 3D cells
    if( getElementDim( typeRegions.first ) == 3 )
    {
      for( auto const & regionCells: typeRegions.second )
      {
        importFieldOnCellElementSubRegion( getLogLevel(),
                                           regionCells.first,
                                           typeRegions.first,
                                           regionCells.second,
                                           elemManager,
                                           m_fieldNamesInGEOSX,
                                           srcArrays,
                                           fieldsToBeSync );
      }
    }
  }

  CommunicationTools::getInstance().synchronizeFields( fieldsToBeSync,
                                                       domain.getMeshBody( this->getName() ).getBaseDiscretization(),
                                                       domain.getNeighbors(),
                                                       false );
}

void RESQMLMeshGenerator::freeResources()
{
  m_vtkMesh = nullptr;
  m_cellMap.clear();
  m_repository = nullptr;
}

std::tuple< string, string > RESQMLMeshGenerator::getParentRepresentation() const
{
  return {m_parent_uuid, m_parent_title};
}

REGISTER_CATALOG_ENTRY( MeshGeneratorBase, RESQMLMeshGenerator, string const &, Group * const )

} // namespace geosx
