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
 * @file RESQMLOutput.cpp
 */

#include "RESQMLOutput.hpp"
#include "mesh/DomainPartition.hpp"
#include "mesh/MeshManager.hpp"
#include "RESQMLMeshGenerator.hpp"



namespace geos
{

using namespace dataRepository;

RESQMLOutput::RESQMLOutput( string const & name,
                            Group * const parent ):
  OutputBase( name, parent )
  , m_plotFileName( name )
  , m_plotLevel()
  , m_onlyPlotSpecifiedFieldNames()
  , m_fieldNames( )
  , m_referenceObjectName( )
  , m_writer( getOutputDirectory() )
{
  registerWrapper( viewKeysStruct::plotFileName, &m_plotFileName ).
    setDefaultValue( m_plotFileName ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "Name of the file for this output." );

  registerWrapper( viewKeysStruct::plotLevel, &m_plotLevel ).
    setApplyDefaultValue( 1 ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "Level detail plot. Only fields with lower of equal plot level will be output." );

  registerWrapper( viewKeysStruct::onlyPlotSpecifiedFieldNames, &m_onlyPlotSpecifiedFieldNames ).
    setApplyDefaultValue( 0 ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription(
    "If this flag is equal to 1, then we only plot the fields listed in `fieldNames`. Otherwise, we plot all the fields with the required `plotLevel`, plus the fields listed in `fieldNames`" );

  registerWrapper( viewKeysStruct::fieldNames, &m_fieldNames ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "Names of the fields to output. If this attribute is specified, GEOS outputs all the fields specified by the user, regardless of their `plotLevel`" );

  registerWrapper( viewKeysStruct::inputRepositoryName, &m_inputRepositoryName ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "The name of the input Data Object Repository from which to retrieve data." );

  registerWrapper( viewKeysStruct::referenceObjectName, &m_referenceObjectName ).
    setInputFlag( InputFlags::OPTIONAL ).
    setDescription( "The name of the object from which to retrieve field values." );

}

RESQMLOutput::~RESQMLOutput()
{}

void RESQMLOutput::postInputInitialization()
{
  m_writer.setOutputLocation( getOutputDirectory(), m_plotFileName );
  m_writer.setFieldNames( m_fieldNames.toViewConst() );
  m_writer.setOnlyPlotSpecifiedFieldNamesFlag( m_onlyPlotSpecifiedFieldNames );

//SupportingRepresentation

  // Use the information of the parent grid provided by the user
  // if(!m_parentMeshUUID.empty() && !m_parentMeshName.empty())
  // {
  //   m_writer.setParentRepresentation( { m_parentMeshUUID, m_parentMeshName } );
  // }
  if( !m_referenceObjectName.empty())// or search for a RESQML input grid in the simulation deck to fill the blanks
  {
    MeshManager & meshManager = this->getGroupByPath< MeshManager >( "/Problem/Mesh" );
    RESQMLMeshGenerator * resqmlMeshGenerator = meshManager.getGroupPointer< RESQMLMeshGenerator >( m_referenceObjectName );

    GEOS_THROW_IF( resqmlMeshGenerator == nullptr,
                   getName() << ": RESQMLMesh not found: " << m_referenceObjectName,
                   InputError );

    m_writer.setParentRepresentation( {resqmlMeshGenerator->getUuid(), resqmlMeshGenerator->getTitle() } );
  }
  else
  {
    GEOS_THROW( GEOS_FMT( "{}: You must provide either a RESQMLMesh name or the parent grid Name and UUID", getName() ),
                InputError );
  }

  m_writer.initializeOutput();
}

void RESQMLOutput::reinit()
{
  //m_writer.clearData();
}

bool RESQMLOutput::execute( real64 const time_n,
                            real64 const GEOS_UNUSED_PARAM( dt ),
                            integer const cycleNumber,
                            integer const GEOS_UNUSED_PARAM( eventCounter ),
                            real64 const GEOS_UNUSED_PARAM ( eventProgress ),
                            DomainPartition & domain )
{
  if( cycleNumber == 0 )
  {
    m_writer.generateSubRepresentations( domain );
  }

  // if( cycleNumber == 0 )
  // {
  m_writer.write( time_n, cycleNumber, domain );
  // }

  return false;
}

void RESQMLOutput::cleanup( real64 const GEOS_UNUSED_PARAM( time_n ),
                            integer const GEOS_UNUSED_PARAM( cycleNumber ),
                            integer const GEOS_UNUSED_PARAM( eventCounter ),
                            real64 const GEOS_UNUSED_PARAM( eventProgress ),
                            DomainPartition & GEOS_UNUSED_PARAM( domain ) )
{
  if( MpiWrapper::commRank( ) == 0 )
  {
    m_writer.generateOutput();
  }
}

REGISTER_CATALOG_ENTRY( OutputBase, RESQMLOutput, string const &, Group * const )

} /* namespace geos */
