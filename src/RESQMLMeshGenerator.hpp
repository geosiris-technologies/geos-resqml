/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2020 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2020 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2020 Total, S.A
 * Copyright (c) 2019-     GEOS Contributors
 * All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

/**
 * @file RESQMLMeshGenerator.hpp
 */

#ifndef GEOS_EXTERNALCOMPONENTS_RESQML_RESQMLMESHGENERATOR_HPP
#define GEOS_EXTERNALCOMPONENTS_RESQML_RESQMLMESHGENERATOR_HPP

// #include "codingUtilities/StringUtilities.hpp"
// #include "codingUtilities/Utilities.hpp"
// #include "mesh/ElementType.hpp"
#include "mesh/generators/ExternalMeshGeneratorBase.hpp"
#include "mesh/generators/VTKUtilities.hpp"
// #include "mesh/FieldIdentifiers.hpp"
#include <vtkDataSet.h>

#include "fesapi/common/EpcDocument.h"

namespace geos
{

class EnergyMLDataObjectRepository;

/**
 *  @class RESQMLMeshGenerator
 *  @brief The RESQMLMeshGenerator class provides a class implementation of RESQML generated meshes from the Fesapi library.
 */
class RESQMLMeshGenerator : public ExternalMeshGeneratorBase
{
public:

  /**
   * @brief Main constructor for MeshGenerator base class.
   * @param[in] name of the RESQMLMeshGenerator object
   * @param[in] parent the parent Group pointer for the MeshGenerator object
   */
  RESQMLMeshGenerator( const string & name,
                       Group * const parent );

  /**
   * @brief Return the name of the RESQMLMeshGenerator in object Catalog.
   * @return string that contains the key name to RESQMLMeshGenerator in the Catalog
   */
  static string catalogName() { return "RESQMLMesh"; }


  Group * createChild( string const & childKey, string const & childName ) override;

  void expandObjectCatalogs() override;


  /**
   * @brief Generate the mesh using fesapi library
   * 
   * @param[inout] cellBlockManager the CellBlockManager that will receive the meshing information
   * @param[in] partition the number of domain in each direction (x,y,z) for InternalMesh only, not used here
   * @details blabla
   */
  void fillCellBlockManager( CellBlockManager & cellBlockManager, array1d< int > const & partition ) override;

  void importFieldOnArray( Block block,
                           string const & blockName,
                           string const & meshFieldName,
                           bool isMaterialField,
                           dataRepository::WrapperBase & wrapper ) const override;
                           
  /**
   * @brief Free the memory of the temporary objects used to load the file.
   */
  void freeResources() override;

  /**
   * @brief Get the Parent Representation object
   * @return a tuple (uuid, name) of the loaded representation
   */
  std::tuple< string, string > getParentRepresentation() const;


  const std::string& getTitle() const { return m_title; }
  const std::string& getUuid() const { return m_uuid; }

protected:

  /**
   * @brief Deserializes the RESQML epc file into a Data Repository.
   * This repository is then used to populate the vtk data structures.
   */
  void postProcessInput() override;

private:

  ///@cond DO_NOT_DOCUMENT
  struct viewKeyStruct
  {
    constexpr static char const * repositoryString() { return "repositoryName"; }
    constexpr static char const * uuidString() { return "uuid"; }
    constexpr static char const * titleString() { return "title"; }    
    constexpr static char const * regionAttributeString() { return "regionAttribute"; }
    // constexpr static char const * uuidsFieldsToImportString() { return "UUIDsFieldsToImport"; }
    // constexpr static char const * fieldsToImportString() { return "fieldsToImport"; }    
    // constexpr static char const * uuidsSurfacesToImportString() { return "UUIDsSurfacesToImport"; }
    constexpr static char const * nodesetNamesString() { return "nodesetNames"; }
    constexpr static char const * partitionRefinementString() { return "partitionRefinement"; }
    constexpr static char const * partitionMethodString() { return "partitionMethod"; }
    constexpr static char const * useGlobalIdsString() { return "useGlobalIds"; }
  };

  struct groupKeyStruct
  {
    static constexpr char const * regionString() { return "Region"; }
    static constexpr char const * propertyString() { return "Property"; }
  };
  /// @endcond


  /**
   * @brief Generate the mesh using fesapi library for reading RESQML data
   * @param[in] domain in the DomainPartition to be written
   * @details This method leverages the fesapi library to load meshes into
   * vtk data structures.
   */
  void generateMesh( DomainPartition & domain );

  /**
   * @brief 
   * 
   * @param cellBlockName 
   * @param meshFieldName 
   * @param isMaterialField 
   * @param wrapper 
   */
  void importVolumicFieldOnArray( string const & cellBlockName, string const & meshFieldName, bool isMaterialField, WrapperBase & wrapper ) const;


  /**
   * @brief Load a list of surfaces from fesapi into CellData of
   * a vtkDataSet
   * @param[in] mesh The dataset in which load the surfaces
   * @return the dataset with the surfaces
   */
  vtkSmartPointer< vtkDataSet > loadSurfaces( vtkSmartPointer< vtkDataSet > mesh );

  /**
   * @brief Load a list of regions from fesapi into CellData of
   * a vtkDataSet
   * @param[in] mesh The dataset in which load the regions
   * @return the dataset with the regions
   */
  vtkSmartPointer< vtkDataSet > loadRegions( vtkSmartPointer< vtkDataSet > mesh );

  /**
   * @brief Load a list of fields from fesapi into CellData of
   * a vtkDataSet
   * @param[in] mesh The dataset in which load the fields
   * @return the dataset with the fields
   */
  vtkSmartPointer< vtkDataSet > loadProperties( vtkSmartPointer< vtkDataSet > mesh );

  /**
   * @brief Looking for the UnstructuredGrid with fesapi and
   * Load DataObject into a vtkDataSet
   * @return the loaded object into a dataset
   */
  vtkSmartPointer< vtkDataSet > retrieveUnstructuredGrid();

  /**
   * @brief Load the RESQML data into the VTK data structure
   * @return a vtk mesh
   */
  vtkSmartPointer< vtkDataSet > loadMesh();


  ///Repository of RESQML objects
  EnergyMLDataObjectRepository * m_repository;
  string m_objectName;

  /// UUID and title of the mesh
  string m_uuid;
  string m_title;


  string_array m_regions;

  string_array m_properties;
  
  /**
   * @brief The VTK mesh to be imported into GEOS.
   * @note We keep this smart pointer as a member for use in @p importFields().
   */
  vtkSmartPointer< vtkDataSet > m_vtkMesh;

  /// UUIDs of the subrepresentation to import as regions
  string_array m_uuidsRegionsToImport;

  /// Name of VTK dataset attribute used to mark regions
  string m_attributeName;

  /// UUIDs of the fields to import
  string_array m_uuidsFieldsToImport;

  // Name of the fields to import
  string_array m_fieldsToImport;

  /// UUIDs of the surfaces to import
  string_array m_uuidsSurfacesToImport;

  /// Names of VTK nodesets to import
  string_array m_nodesetNames;

  /// Number of graph partitioning refinement iterations
  integer m_partitionRefinement = 0;

  /// Whether global id arrays should be used, if available
  integer m_useGlobalIds = 0;

  /// Method (library) used to partition the mesh
  vtk::PartitionMethod m_partitionMethod = vtk::PartitionMethod::parmetis;

  /// Lists of VTK cell ids, organized by element type, then by region
  vtk::CellMapType m_cellMap;
};

} // namespace geos

#endif /* GEOS_EXTERNALCOMPONENTS_RESQML_RESQMLMESHGENERATOR_HPP */