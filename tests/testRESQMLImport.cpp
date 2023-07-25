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


// Source includes
#include "codingUtilities/UnitTestUtilities.hpp"
#include "dataRepository/xmlWrapper.hpp"
#include "mainInterface/GeosxState.hpp"
#include "mainInterface/initialization.hpp"
#include "mesh/MeshManager.hpp"
#include "mesh/generators/CellBlockManagerABC.hpp"
#include "mesh/generators/CellBlockABC.hpp"

// special CMake-generated include
#include "tests/meshDirName.hpp"

// TPL includes
#include <gtest/gtest.h>
#include <conduit.hpp>

using namespace geos;
using namespace geos::testing;
using namespace geos::dataRepository;

void TestRESQMLMeshNode( string const & meshNode )
{
  xmlWrapper::xmlDocument xmlDocument;
  xmlDocument.load_buffer( meshNode.c_str(), meshNode.size() );
  xmlWrapper::xmlNode xmlMeshNode = xmlDocument.child( "Mesh" );

  conduit::Node node;
  Group root( "root", node );

  MeshManager meshManager( "mesh", &root );
  meshManager.processInputFileRecursive( xmlMeshNode );
  meshManager.postProcessInputRecursive();
  DomainPartition domain( "domain", &root );
  meshManager.generateMeshes( domain );
}

template< class V >
void TestMeshImport( string const & meshFilePath, V const & validate )
{
  string const meshNode = GEOS_FMT( R"(<Mesh><RESQMLMesh name="mesh" UUID="9283cd33-5e52-4110-b7b1-616abde2b303" file="{}"/></Mesh>)", meshFilePath );
  xmlWrapper::xmlDocument xmlDocument;
  xmlDocument.load_buffer( meshNode.c_str(), meshNode.size() );
  xmlWrapper::xmlNode xmlMeshNode = xmlDocument.child( "Mesh" );

  conduit::Node node;
  Group root( "root", node );

  MeshManager meshManager( "mesh", &root );
  meshManager.processInputFileRecursive( xmlMeshNode );
  meshManager.postProcessInputRecursive();
  DomainPartition domain( "domain", &root );
  meshManager.generateMeshes( domain );

  validate( domain.getMeshBody( "mesh" ).getGroup< CellBlockManagerABC >( keys::cellManager ) );
}

TEST( RESQMLNode, meshName )
{
  string const meshFilePath = testMeshDir + "/testingPackageCpp.epc";
  string const meshName = "One tetrahedron + prism grid";
  string const meshUUID = "9283cd33-5e52-4110-b7b1-616abde2b303";
  {
    string const meshNode = GEOS_FMT( R"(<Mesh><RESQMLMesh name="{}" UUID="{}" file="{}"/></Mesh>)", meshName, meshUUID, meshFilePath );
    TestRESQMLMeshNode( meshNode );
  }

  // {
  //   string const region1 = "323001d0-468c-41d7-abec-7d12c3c9428b";
  //   string const region2 = "f6d23b9c-e45d-4638-9601-ae3b682129a0";
  //   string const meshNode = GEOSX_FMT( R"(<Mesh><RESQMLMesh name="{}" UUID="{}" UUIDsRegionsToImport="{{{},{}}}" file="{}"/></Mesh>)", meshName, meshUUID, region1, region2, meshFilePath );
  //   TestRESQMLMeshNode( meshNode );
  // }
  //test avec region: 
  //test region automatiques
  // {
  //   string const meshNode = GEOSX_FMT( R"(<Mesh><RESQMLMesh name="{}" UUID="{}" file="{}"/></Mesh>)", meshName, meshUUID, meshFilePath );
  //   TestRESQMLMeshNode( meshNode );
  // }

}

TEST( RESQMLImport, testingPackage )
{
  auto validate = []( CellBlockManagerABC const & cellBlockManager ) -> void
  {
    // `testingPackage.epc` is made of 2 elements.
    // - Element 0 is a tetrahedron.
    // - Element 1 is a prism.
    // It contains 7 nodes, 12 edges, 8 faces.
    ASSERT_EQ( cellBlockManager.numNodes(), 7 );
    ASSERT_EQ( cellBlockManager.numEdges(), 12 );
    ASSERT_EQ( cellBlockManager.numFaces(), 8 );
    ASSERT_EQ( cellBlockManager.numSubGroups(), 2 );
    SortedArray< localIndex > const & allNodes = cellBlockManager.getNodeSets().at( "all" );
    ASSERT_EQ( allNodes.size(), 7 );
  };

  string const epc = testMeshDir + "/testingPackageCpp.epc";
  TestMeshImport( epc, validate );
}


int main( int argc, char * * argv )
{
  ::testing::InitGoogleTest( &argc, argv );

  geos::GeosxState state( geos::basicSetup( argc, argv ) );

  int const result = RUN_ALL_TESTS();

  geos::basicCleanup();

  return result;
}
