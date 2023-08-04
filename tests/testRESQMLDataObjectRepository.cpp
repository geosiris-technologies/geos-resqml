#include "codingUtilities/UnitTestUtilities.hpp"
#include "dataRepository/xmlWrapper.hpp"
#include "mainInterface/ProblemManager.hpp"
#include "mainInterface/GeosxState.hpp"
#include "mainInterface/initialization.hpp"
#include "mesh/generators/RESQML/RESQMLDataObjectRepository.hpp"

// special CMake-generated include
#include "tests/meshDirName.hpp"

// TPL includes
#include <gtest/gtest.h>
#include <conduit.hpp>

using namespace geosx;
using namespace geosx::testing;
using namespace geosx::dataRepository;


void TestEpcDocument( string const & epcFilePath)
{
    string const epcNode = GEOSX_FMT( 
        "<Problem>"
        "   <RESQMLDataObjectRepository>"
        "      <EpcDocumentRepository name=\"repo\" file=\"{}\"/>"
        "   </RESQMLDataObjectRepository>"
        "</Problem>",
        epcFilePath);
    xmlWrapper::xmlDocument xmlDocument;
    xmlWrapper::xmlResult xmlResult = xmlDocument.load_buffer( epcNode.c_str(), epcNode.size() );
    
    ASSERT_TRUE( xmlResult );

    xmlWrapper::xmlNode xmlProblemNode = xmlDocument.child( dataRepository::keys::ProblemManager );
    ProblemManager & problemManager = getGlobalState().getProblemManager();
    problemManager.processInputFileRecursive( xmlProblemNode );

    problemManager.printDataHierarchy(3);

    // Open DataObject Repository levels
    // RESQMLDataObjectRepository & repo = problemManager.getGroup< RESQMLDataObjectRepository >( problemManager.groupKeys.resqmlDataObjectRepository );

    // repo.test();
    // repo.processInputFileRecursive( xmlMeshNode );
    // repo.postProcessInputRecursive();
    // DomainPartition domain( "domain", &root );
    // meshManager.generateMeshes( domain );

    // validate( domain.getMeshBody( "mesh" ).getGroup< CellBlockManagerABC >( keys::cellManager ) );
}

TEST( EpcDocument, sizes )
{

    string const test_epc = testMeshDir + "/test.epc";
    TestEpcDocument( test_epc );
}

int main( int argc, char * * argv )
{
  ::testing::InitGoogleTest( &argc, argv );

  geosx::GeosxState state( geosx::basicSetup( argc, argv ) );

  int const result = RUN_ALL_TESTS();

  geosx::basicCleanup();

  return result;
}
