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
 * @file Surface.cpp
 */

#include "Surface.hpp"

namespace geos
{
using namespace dataRepository;

Surface::Surface( string const & name,
                  Group * const parent )
  :
  Group( name, parent )
{
  setInputFlags( InputFlags::OPTIONAL );
  enableLogLevelInput();

  registerWrapper( viewKeyStruct::regionIdString(), &m_regionId ).
    setInputFlag( InputFlags::REQUIRED ).
    setDescription( "Region Id of the surface" );

  registerWrapper( viewKeyStruct::uuidString(), &m_uuid ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "UUID of the data object" );

  registerWrapper( viewKeyStruct::titleString(), &m_title ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "Title of the data object" );
}

void Surface::postInputInitialization()
{
  //test region->getElementKindOfPatch( 0, 0 ) != gsoap_eml2_3::eml23__IndexableElement::faces
}


Surface::CatalogInterface::CatalogType & Surface::getCatalog()
{
  static Surface::CatalogInterface::CatalogType catalog;
  return catalog;
}

REGISTER_CATALOG_ENTRY( Group, Surface, string const &, Group * const )

} // namespace geos
