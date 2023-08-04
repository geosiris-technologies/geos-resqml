
#include "mesh/generators/RESQML/ETPRepository.hpp"

namespace geosx
{

using namespace dataRepository;

ETPRepository::ETPRepository( string const & name,
                                                      dataRepository::Group * const parent )
  : RESQMLDataObjectRepository( name, parent )
{
    enableLogLevelInput();

    registerWrapper( viewKeyStruct:ipConnectionString(), &m_ipConnection ).
    setInputFlag( InputFlags::REQUIRED ).
    setRestartFlags( RestartFlags::NO_WRITE ).
    setDescription( "IP from the ETP Server" );

    registerWrapper( viewKeyStruct::portConnectionInt(), &m_portConnection ).
    setInputFlag( InputFlags::REQUIRED ).
    setRestartFlags( RestartFlags::NO_WRITE ).
    setDescription( "PORT from the ETP Server" );

    registerWrapper( viewKeyStruct::authConnectionString(), &m_authConnection ).
    setInputFlag( InputFlags::REQUIRED ).
    setRestartFlags( RestartFlags::NO_WRITE ).
    setDescription( "Authentification for the connection to the ETP Server" );
}

void ETPRepository::postProcessInput()
{
	boost::uuids::random_generator gen;
	ETP_NS::InitializationParameters initializationParams(gen(), ip_connection, port_connection);

	m_session = ETP_NS::ClientSessionLaunchers::createWsClientSession(&initializationParams, "/", auth_connection);
	m_session->setCoreProtocolHandlers(std::make_shared<FesppCoreProtocolHandlers>(session.get(), repository));
	m_session->setDiscoveryProtocolHandlers(std::make_shared<FesppDiscoveryProtocolHandlers>(session.get(), repository));
	auto storeHandlers = std::make_shared<FesppStoreProtocolHandlers>(session.get(), repository);
	session->setStoreProtocolHandlers(storeHandlers);
	session->setDataArrayProtocolHandlers(std::make_shared<ETP_NS::DataArrayHandlers>(session.get()));

	repository->setHdfProxyFactory(new ETP_NS::FesapiHdfProxyFactory(session.get()));

	std::thread sessionThread(&ETP_NS::PlainClientSession::run, session);
	sessionThread.detach();
	while (!storeHandlers->isDone()) {}
}

} // end namespace geosx