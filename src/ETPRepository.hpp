#ifndef GEOSX_MESH_GENERATORS_RESQML_ETPREPOSITORY_HPP
#define GEOSX_MESH_GENERATORS_RESQML_ETPREPOSITORY_HPP

#include "EnergyMLDataObjectRepository.hpp"

// #include <fetpapi/etp/ClientSessionLaunchers.h>

namespace geos
{

class ETPRepository : public EnergyMLDataObjectRepository
{
public:

  /**
   * @brief Constructor.
   * @param[in] name name of the object
   * @param[in] parent the parent Group pointer
   */
  ETPRepository( const string & name,
                 Group * const parent );

  /**
   * @brief Return the name of the ETPRepository in object catalog.
   * @return string that contains the catalog name of the ETPRepository
   */
  static string catalogName() { return "ETPRepository"; }

  void open() override;

protected:

  ///@cond DO_NOT_DOCUMENT
  struct viewKeyStruct
  {
    constexpr static char const * ipConnectionString() { return "ip"; }
    constexpr static char const * portConnectionInt() { return "port"; }
    constexpr static char const * authConnectionString() { return "auth_connection"; }
  };
  /// @endcond

  void postInputInitialization() override;

private:
  /// ETP Session
  // std::shared_ptr<ETP_NS::PlainClientSession> m_session;

  ///
  string m_ipConnection;

  ///
  integer m_portConnection;

  ///
  string m_authConnection;
};

} // end namespace geosx

#endif /* GEOSX_MESH_GENERATORS_RESQML_ETPREPOSITORY_HPP */
