
#include "EnergyMLDataObjectRepository.hpp"
#include "mesh/ExternalDataRepositoryManager.hpp"

#include <fesapi/resqml2/UnstructuredGridRepresentation.h>


#include <algorithm>

namespace geos
{

using namespace dataRepository;

EnergyMLDataObjectRepository::EnergyMLDataObjectRepository( string const & name,
                                                            dataRepository::Group * const parent ):
  ExternalDataRepositoryBase( name, parent ),
  m_repository( new common::DataObjectRepository())
{
  setInputFlags( InputFlags::OPTIONAL_NONUNIQUE );

  // This enables logLevel filtering
  enableLogLevelInput();
}

Group * EnergyMLDataObjectRepository::createChild( string const & childKey, string const & childName )
{
  // RESQML DataObject Repositories generally don't have child XML nodes, must override this method to enable
  GEOS_THROW( GEOS_FMT( "EnergyMLDataObjectRepository '{}': invalid child XML node '{}' of type {}", getName(), childName, childKey ),
              InputError );
  std::unique_ptr< EnergyMLDataObjectRepository > task = EnergyMLDataObjectRepository::CatalogInterface::factory( childKey, childName, this );
  return &this->getParent().registerGroup< EnergyMLDataObjectRepository >( childName, std::move( task ) );

}

EnergyMLDataObjectRepository::CatalogInterface::CatalogType & EnergyMLDataObjectRepository::getCatalog()
{
  static EnergyMLDataObjectRepository::CatalogInterface::CatalogType catalog;
  return catalog;
}

common::DataObjectRepository * EnergyMLDataObjectRepository::getData()
{
  return m_repository;
}

COMMON_NS::AbstractObject * EnergyMLDataObjectRepository::getDataObject( string const & id )
{
  return m_repository->getDataObjectByUuid( id );
}

COMMON_NS::AbstractObject * EnergyMLDataObjectRepository::getDataObjectByTitle( string const & name )
{
  for( auto & [key, value] : m_repository->getDataObjects())
  {
    //look at the different version of the dataObject
    for( auto * dataObject : value )
    {
      if( dataObject->getTitle() == name )
      {
        return dataObject;
      }
    }
  }

  return nullptr;
}


RESQML2_NS::UnstructuredGridRepresentation * EnergyMLDataObjectRepository::retrieveUnstructuredGrid( string const & id )
{
  return m_repository->getDataObjectByUuid< RESQML2_NS::UnstructuredGridRepresentation >( id );
}

RESQML2_NS::UnstructuredGridRepresentation * EnergyMLDataObjectRepository::retrieveUnstructuredGridByTitle( string const & title )
{
  RESQML2_NS::UnstructuredGridRepresentation * rep{nullptr};

  auto grid_set = m_repository->getUnstructuredGridRepresentationSet();
  auto result = std::find_if( std::begin( grid_set ), std::end( grid_set ), [&title]( RESQML2_NS::UnstructuredGridRepresentation *grid ) {
    return grid->getTitle() == title;
  } );

  if( result != std::end( grid_set ))
  {
    rep = *result;
  }

  return rep;
}

} // end namespace geosx
