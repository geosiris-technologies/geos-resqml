
#ifndef GEOSX_MESH_GENERATORS_RESQML_RESQMLDATAOBJECTREPOSITORY_HPP
#define GEOSX_MESH_GENERATORS_RESQML_RESQMLDATAOBJECTREPOSITORY_HPP

#include "mesh/generators/MeshGeneratorBase.hpp"

#include "fesapi/common/DataObjectRepository.h"

namespace geosx
{

class RESQMLDataObjectRepository : public MeshGeneratorBase
{
public:

  /**
   * @brief Default constructor.
   */
  RESQMLDataObjectRepository() = delete;


  /**
   * @brief Main constructor for RESQMLDataObjectRepository base class.
   * @param[in] name of the RESQMLDataObjectRepository object
   * @param[in] parent the parent Group pointer for the RESQMLDataObjectRepository object
   */
  explicit RESQMLDataObjectRepository( string const & name,
                              Group * const parent );

  // /**
  //  * @brief Return the name of the MeshGenerator in object catalog.
  //  * @return string that contains the catalog name of the MeshGenerator
  //  */
  // static string catalogName() { return "RESQMLDataObjectRepository"; }

  /// using alias for templated Catalog meshGenerator type
  using CatalogInterface = dataRepository::CatalogInterface< RESQMLDataObjectRepository, string const &, Group * const >;

  /**
   * @brief Accessor for the singleton Catalog object
   * @return a static reference to the Catalog object
   */
  static CatalogInterface::CatalogType & getCatalog();

  /**
   * @brief function to return the catalog name of the derived class
   * @return a string that contains the catalog name of the derived class
   */
  virtual string getCatalogName() = 0;
  // /**
  //  * @brief Create a new object as a child of this group.
  //  * @param childKey the catalog key of the new object to create
  //  * @param childName the name of the new object in the repository
  //  * @return the group child
  //  */
  // virtual Group * createChild( string const & childKey, string const & childName ) override;


protected:

  /// RESQML DataObject Repository
	common::DataObjectRepository * m_repository;

};

} // end namespace

#endif /* GEOSX_MESH_GENERATORS_RESQML_RESQMLDATAOBJECTREPOSITORY_HPP */