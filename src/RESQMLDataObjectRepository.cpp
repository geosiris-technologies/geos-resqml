
#include "RESQMLDataObjectRepository.hpp"

namespace geosx
{

using namespace dataRepository;

RESQMLDataObjectRepository::RESQMLDataObjectRepository(string const & name,
                                                        dataRepository::Group * const parent) :
    MeshGeneratorBase( name, parent )
{
    setInputFlags(InputFlags::OPTIONAL_NONUNIQUE);

    // This enables logLevel filtering
    enableLogLevelInput();
}

// Group * RESQMLDataObjectRepository::createChild( string const & childKey, string const & childName )
// {
//   // RESQML DataObject Repositories generally don't have child XML nodes, must override this method to enable
//   // GEOSX_THROW( GEOSX_FMT( "RESQMLDataObjectRepository '{}': invalid child XML node '{}' of type {}", getName(), childName, childKey ),
//               //  InputError );
//   std::unique_ptr< RESQMLDataObjectRepository > task = RESQMLDataObjectRepository::CatalogInterface::factory( childKey, childName, this );
//   return &this->registerGroup< RESQMLDataObjectRepository >( childName, std::move( task ) );

// }

RESQMLDataObjectRepository::CatalogInterface::CatalogType & RESQMLDataObjectRepository::getCatalog()
{
  static RESQMLDataObjectRepository::CatalogInterface::CatalogType catalog;
  return catalog;
}


} // end namespace geosx