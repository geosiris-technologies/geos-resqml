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
 * @file Surface.hpp
 */

#ifndef GEOS_EXTERNALCOMPONENTS_RESQML_SURFACE_HPP
#define GEOS_EXTERNALCOMPONENTS_RESQML_SURFACE_HPP

#include "dataRepository/Group.hpp"

namespace geos
{

/**
 * @brief Surface parameters with Group capabilities
 *
 * This class is a derived version of LinearSolverParameters with
 * dataRepository::Group capabilities to allow for XML input.
 *
 */

class Surface : public dataRepository::Group
{
public:
  Surface() = delete;

  /// Constructor
  Surface( string const & name, Group * const parent );

  /// Copy constructor
  Surface( Surface && ) = default;

  /**
   * @brief Accessor for the singleton Catalog object
   * @return a static reference to the Catalog object
   */
  static CatalogInterface::CatalogType & getCatalog();

  /// Destructor
  virtual ~Surface() override = default;

  /// Catalog name
  static string catalogName() { return "Surface"; }

  /// Postprocessing of input
  virtual void postInputInitialization() override;

  const string & getUUID() const { return m_uuid; }

  const string & getTitle() const { return m_title; }

  integer getRegionId() const { return m_regionId; }

private:

  /// Keys appearing in XML
  struct viewKeyStruct
  {
    /// Solver type key
    static constexpr char const *uuidString() { return "uuid"; }
    static constexpr char const *titleString() { return "title"; }
    static constexpr char const *regionIdString() { return "regionId"; }
  };

  string m_uuid;
  string m_title;
  integer m_regionId;
};

} // namespace geos

#endif // GEOS_EXTERNALCOMPONENTS_RESQML_SURFACE_HPP
