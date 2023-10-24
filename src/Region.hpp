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
 * @file Region.hpp
 */

#ifndef GEOS_EXTERNALCOMPONENTS_RESQML_REGION_HPP
#define GEOS_EXTERNALCOMPONENTS_RESQML_REGION_HPP

#include "mesh/MeshBase.hpp"

namespace geos {

/**
 * @brief Region parameters with Group capabilities
 *
 * This class is a derived version of LinearSolverParameters with
 * dataRepository::Group capabilities to allow for XML input.
 *
 */

class Region : public MeshBase {
public:
  Region() = delete;

  /// Constructor
  Region(string const &name, Group *const parent);

  /// Copy constructor
  Region(Region &&) = default;

  /**
   * @brief Accessor for the singleton Catalog object
   * @return a static reference to the Catalog object
   */
  static CatalogInterface::CatalogType & getCatalog();

  /// Destructor
  virtual ~Region() override = default;

  /// Catalog name
  static string catalogName() { return "Region"; }

  /// Postprocessing of input
  virtual void postProcessInput() override;

  const string& getUUID() const { return m_uuid; }

private:

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

#endif // GEOS_EXTERNALCOMPONENTS_RESQML_REGION_HPP