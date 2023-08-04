
#include "mesh/generators/RESQML/EpcDocumentRepository.hpp"

namespace geosx
{

using namespace dataRepository;

EpcDocumentRepository::EpcDocumentRepository( string const & name,
                                              Group * const parent )
  : RESQMLDataObjectRepository( name, parent )
{
    enableLogLevelInput();

    registerWrapper( viewKeyStruct::filePathString(), &m_filePath ).
    setInputFlag( InputFlags::REQUIRED ).
    setRestartFlags( RestartFlags::NO_WRITE ).
    setDescription( "Path to the mesh file" );
}

void EpcDocumentRepository::postProcessInput()
{
  GEOSX_LOG_RANK_0( GEOSX_FMT( "Reading: {}", m_filePath) );
	COMMON_NS::EpcDocument pck(m_filePath);
	std::string message = pck.deserializePartiallyInto(*m_repository);
	pck.close();
  
  GEOSX_LOG_RANK_0( GEOSX_FMT( "Deserilization message: {}", message) );
}

REGISTER_CATALOG_ENTRY( RESQMLDataObjectRepository, EpcDocumentRepository, string const &, Group * const )

} // end namespace geosx