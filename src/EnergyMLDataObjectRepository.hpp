
#ifndef GEOSX_MESH_GENERATORS_RESQML_ENERGYMLDATAOBJECTREPOSITORY_HPP
#define GEOSX_MESH_GENERATORS_RESQML_ENERGYMLDATAOBJECTREPOSITORY_HPP

#include "mesh/ExternalDataRepositoryBase.hpp"

#include "fesapi/common/DataObjectRepository.h"

namespace geos
{

class EnergyMLDataObjectRepository : public ExternalDataRepositoryBase
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


  common::DataObjectRepository * getData();

  COMMON_NS::AbstractObject * getDataObject( string const & id );
  COMMON_NS::AbstractObject * getDataObjectByTitle( string const & name );

  // RESQML2_NS::UnstructuredGridRepresentation * retrieveUnstructuredGrid(string const & name);

  // RESQML2_NS::UnstructuredGridRepresentation * retrieveUnstructuredGrid(UUID const & id);
  RESQML2_NS::UnstructuredGridRepresentation * retrieveUnstructuredGrid( string const & id );
  RESQML2_NS::UnstructuredGridRepresentation * retrieveUnstructuredGridByTitle( string const & name );

protected:

  /// RESQML DataObject Repository
  common::DataObjectRepository * m_repository;

};

} // end namespace

#endif /* GEOSX_MESH_GENERATORS_RESQML_EnergyMLDataObjectRepository_HPP */
