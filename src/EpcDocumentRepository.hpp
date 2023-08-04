#ifndef GEOSX_MESH_GENERATORS_RESQML_EPCDOCUMENTREPOSITORY_HPP
#define GEOSX_MESH_GENERATORS_RESQML_EPCDOCUMENTREPOSITORY_HPP

#include "mesh/generators/RESQML/RESQMLDataObjectRepository.hpp"

#include "fesapi/common/EpcDocument.h"

namespace geosx
{

class EpcDocumentRepository : public RESQMLDataObjectRepository
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

  virtual void test() = 0;

protected:
  
  ///@cond DO_NOT_DOCUMENT
  struct viewKeyStruct
  {
    constexpr static char const * filePathString() { return "file"; }
  };
  /// @endcond

  void postProcessInput() override;

private:
  /// Path to the epc file
  Path m_filePath;
};

} // end namespace geosx

#endif /* GEOSX_MESH_GENERATORS_RESQML_EPCDOCUMENTREPOSITORY_HPP */