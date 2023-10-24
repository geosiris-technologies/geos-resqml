
#ifndef GEOSX_MESH_GENERATORS_RESQML_ENERGYMLDATAOBJECTREPOSITORY_HPP
#define GEOSX_MESH_GENERATORS_RESQML_ENERGYMLDATAOBJECTREPOSITORY_HPP

#include "mesh/MeshBase.hpp"

#include "fesapi/common/DataObjectRepository.h"

namespace geos
{

class EnergyMLDataObjectRepository : public MeshBase
{
public:

  /**
   * @brief Default constructor.
   */
  EnergyMLDataObjectRepository() = delete;


  /**
   * @brief Main constructor for EnergyMLDataObjectRepository base class.
   * @param[in] name of the EnergyMLDataObjectRepository object
   * @param[in] parent the parent Group pointer for the EnergyMLDataObjectRepository object
   */
  explicit EnergyMLDataObjectRepository( string const & name,
                              Group * const parent );

  /**
   * @brief Return the name of the MeshGenerator in object catalog.
   * @return string that contains the catalog name of the MeshGenerator
   */
  static string catalogName() { return "EnergyMLDataObjectRepository"; }

  /// using alias for templated Catalog meshGenerator type
  using CatalogInterface = dataRepository::CatalogInterface< EnergyMLDataObjectRepository, string const &, Group * const >;

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
  virtual Group * createChild( string const & childKey, string const & childName ) override;

  common::DataObjectRepository * getData();


  COMMON_NS::AbstractObject* getDataObject(string const & id);
  COMMON_NS::AbstractObject* getDataObjectByTitle(string const & name);

  // RESQML2_NS::UnstructuredGridRepresentation * retrieveUnstructuredGrid(string const & name);

  // RESQML2_NS::UnstructuredGridRepresentation * retrieveUnstructuredGrid(UUID const & id);
  RESQML2_NS::UnstructuredGridRepresentation * retrieveUnstructuredGrid(string const & id);
  RESQML2_NS::UnstructuredGridRepresentation * retrieveUnstructuredGridByTitle(string const & name);

protected:

  /// RESQML DataObject Repository
	common::DataObjectRepository * m_repository;

};

} // end namespace

#endif /* GEOSX_MESH_GENERATORS_RESQML_EnergyMLDataObjectRepository_HPP */