/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2020 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2020 The Board of Trustees of the Leland Stanford Junior
 * University Copyright (c) 2018-2020 TotalEnergies Copyright (c) 2019- GEOSX
 * Contributors All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS
 * files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

/**
 * @file Property.hpp
 */

#ifndef GEOS_EXTERNALCOMPONENTS_RESQML_PROPERTY_HPP
#define GEOS_EXTERNALCOMPONENTS_RESQML_PROPERTY_HPP

#include "mesh/MeshBase.hpp"

namespace geos {

/**
 * @brief Linear solver parameters with Group capabilities
 *
 * This class is a derived version of LinearSolverParameters with
 * dataRepository::Group capabilities to allow for XML input.
 */

class Property : public MeshBase {
public:
  Property() = delete;

  /// Constructor
  Property(string const &name, Group *const parent);

  /// Copy constructor
  Property(Property &&) = default;
  
  /**
   * @brief Accessor for the singleton Catalog object
   * @return a static reference to the Catalog object
   */
  static CatalogInterface::CatalogType & getCatalog();

  /// Destructor
  virtual ~Property() override = default;

  /// Catalog name
  static string catalogName() { return "Property"; }

  /// Postprocessing of input
  virtual void postProcessInput() override;

  const string& getUUID() const { return m_uuid; }

  const string& getTitle() const { return m_title; }

  //   LinearSolverParameters const & get() const
  //   { return m_parameters; }

  //   LinearSolverParameters & get()
  //   { return m_parameters; }

  /// Keys appearing in XML
  struct viewKeyStruct {
    /// Solver type key
    static constexpr char const *uuidString() { return "uuid"; }
    static constexpr char const *titleString() { return "title"; }
  };

  string m_uuid;
  string m_title;
};

} // namespace geos

#endif // GEOS_EXTERNALCOMPONENTS_RESQML_PROPERTY_HPP