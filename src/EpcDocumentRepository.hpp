#ifndef GEOSX_MESH_GENERATORS_RESQML_EPCDOCUMENTREPOSITORY_HPP
#define GEOSX_MESH_GENERATORS_RESQML_EPCDOCUMENTREPOSITORY_HPP

#include "EnergyMLDataObjectRepository.hpp"

#include "fesapi/common/EpcDocument.h"

namespace geos
{

class EpcDocumentRepository : public EnergyMLDataObjectRepository
{
public:

  /**
   * @brief Constructor.
   * @param[in] name name of the object
   * @param[in] parent the parent Group pointer
   */
  explicit EpcDocumentRepository( const string & name,
                                  Group * const parent );


  /**
   * @brief Return the name of the RESQMLDataObjectRepository in object catalog.
   * @return string that contains the catalog name of the RESQMLDataObjectRepository
   */
  static string catalogName() { return "EpcDocumentRepository"; }

  string getCatalogName() override { return catalogName(); }

  void open() override;

protected:

  ///@cond DO_NOT_DOCUMENT
  struct viewKeyStruct
  {
    constexpr static char const * filesPathsString() { return "files"; }
  };
  /// @endcond

  void postInputInitialization() override;

private:
  /// Path to the epc file
  array1d< Path > m_filesPaths;
};

} // end namespace geosx

#endif /* GEOSX_MESH_GENERATORS_RESQML_EPCDOCUMENTREPOSITORY_HPP */
