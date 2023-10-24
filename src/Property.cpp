/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2020 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2020 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2020 TotalEnergies
 * Copyright (c) 2019-     GEOSX Contributors
 * All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

/**
 * @file Property.cpp
 */

#include "Property.hpp"

namespace geos
{
using namespace dataRepository;

Property::Property( string const & name,
                    Group * const parent )
  :
  MeshBase( name, parent )
{
  setInputFlags( InputFlags::OPTIONAL );
  enableLogLevelInput();

  registerWrapper( viewKeyStruct::uuidString(), &m_uuid ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "UUID of the data object" );

  registerWrapper( viewKeyStruct::titleString(), &m_title ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "Title of the data object" );
}

void Property::postProcessInput()
{
}

Property::CatalogInterface::CatalogType & Property::getCatalog()
{
  static Property::CatalogInterface::CatalogType catalog;
  return catalog;
}

REGISTER_CATALOG_ENTRY( MeshBase, Property, string const &, Group * const )

} // namespace geos
