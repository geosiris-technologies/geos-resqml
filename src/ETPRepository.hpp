#ifndef GEOSX_MESH_GENERATORS_RESQML_ETPREPOSITORY_HPP
#define GEOSX_MESH_GENERATORS_RESQML_ETPREPOSITORY_HPP

#include "mesh/generators/RESQML/RESQMLDataObjectRepository.hpp"

#include <fetpapi/etp/ClientSessionLaunchers.h>

namespace geosx
{

class ETPRepository : public RESQMLDataObjectRepository
{
public:

  /**
   * @brief Constructor.
   * @param[in] name name of the object
   * @param[in] parent the parent Group pointer
   */
  ETPRepository( const string & name,
                 Group * const parent );
    

protected:

  ///@cond DO_NOT_DOCUMENT
  struct viewKeyStruct
  {
    constexpr static char const * ipConnectionString() { return "ip"; }
    constexpr static char const * portConnectionInt() { return "port"; }
    constexpr static char const * authConnectionString() { return "auth_connection"; }
  };
  /// @endcond

  void postProcessInput() override;

private:
    /// ETP Session
    std::shared_ptr<ETP_NS::PlainClientSession> m_session;

    ///
    string m_ipConnection;

    ///
    integer m_portConnection;

    ///
    string m_authConnection;
};

} // end namespace geosx

#endif /* GEOSX_MESH_GENERATORS_RESQML_ETPREPOSITORY_HPP */