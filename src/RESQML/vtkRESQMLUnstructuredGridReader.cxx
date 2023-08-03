
#include "vtkRESQMLUnstructuredGridReader.h"



#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkDataObject.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <vtkDoubleArray.h>

#include <vtkUnstructuredGrid.h>
#include <vtkPolyhedron.h>
#include <vtkTetra.h>
#include <vtkHexagonalPrism.h>
#include <vtkPentagonalPrism.h>
#include <vtkHexahedron.h>
#include <vtkPyramid.h>
#include <vtkWedge.h>

#include <array>

vtkStandardNewMacro( vtkRESQMLUnstructuredGridReader )

vtkRESQMLUnstructuredGridReader::vtkRESQMLUnstructuredGridReader()
{
  this->SetNumberOfInputPorts( 0 );
  this->SetNumberOfOutputPorts( 1 );
}

vtkRESQMLUnstructuredGridReader::~vtkRESQMLUnstructuredGridReader()
{}

//----------------------------------------------------------------------------
void vtkRESQMLUnstructuredGridReader::PrintSelf( ostream & os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );

  os << indent << "Uuid: "
     << this->UnstructuredGridRepresentation->getUuid() << "\n"
     << indent << "Node count: "
     << this->UnstructuredGridRepresentation->getNodeCount() << "\n"
     << indent << "Face count: "
     << this->UnstructuredGridRepresentation->getFaceCount() << "\n"
     << indent << "Cell count: "
     << this->UnstructuredGridRepresentation->getCellCount() << "\n";
}

int vtkRESQMLUnstructuredGridReader::RequestData(
  vtkInformation *vtkNotUsed( request ),
  vtkInformationVector * *vtkNotUsed( inputVector ),
  vtkInformationVector *outputVector )
{
  // get the info objects
  vtkInformation * outInfo = outputVector->GetInformationObject( 0 );

  // get the output
  vtkUnstructuredGrid * output =
    vtkUnstructuredGrid::SafeDownCast( outInfo->Get( vtkDataObject::DATA_OBJECT()));

  output->Allocate( this->UnstructuredGridRepresentation->getCellCount());

  this->buildUnstructuredGrid( output );

  vtkDebugMacro( << "Extracted " << output->GetNumberOfPoints() << " points,"
                 << output->GetNumberOfCells() << " cells." );

  return 1;
}

void vtkRESQMLUnstructuredGridReader::buildUnstructuredGrid( vtkSmartPointer< vtkUnstructuredGrid > vtk_unstructuredGrid )
{
  uint64_t pointCount = this->UnstructuredGridRepresentation->getXyzPointCountOfAllPatches();

  // POINTS
  double *allXyzPoints = new double[pointCount * 3]; // Will be deleted by VTK;
  this->UnstructuredGridRepresentation->getXyzPointsOfAllPatches( allXyzPoints ); //getXyzPointsOfAllPatchesInGlobalCrs

  const size_t coordCount = pointCount * 3;

  if( this->UnstructuredGridRepresentation->getLocalCrs( 0 ) != nullptr )
  {
    if( !this->UnstructuredGridRepresentation->getLocalCrs( 0 )->isPartial())
    {
      const double zIndice = this->UnstructuredGridRepresentation->getLocalCrs( 0 )->isDepthOriented() ? -1 : 1;

      for( size_t zCoordIndex = 2; zCoordIndex < coordCount; zCoordIndex += 3 )
      {
        allXyzPoints[zCoordIndex] *= zIndice;
      }
    }
  }

  vtkNew< vtkDoubleArray > vtkUnderlyingArray;
  vtkUnderlyingArray->SetNumberOfComponents( 3 );
  // Take ownership of the underlying C array
  vtkUnderlyingArray->SetArray( allXyzPoints, coordCount, vtkAbstractArray::VTK_DATA_ARRAY_DELETE );

  vtkNew< vtkPoints > vtkPts;
  vtkPts->SetData( vtkUnderlyingArray );

  vtk_unstructuredGrid->SetPoints( vtkPts );
  this->UnstructuredGridRepresentation->loadGeometry();
  // CELLS
  const unsigned int nodeCount = this->UnstructuredGridRepresentation->getNodeCount();
  std::unique_ptr< vtkIdType[] > pointIds( new vtkIdType[nodeCount] );
  for( unsigned int i = 0; i < nodeCount; ++i )
  {
    pointIds[i] = i;
  }
  const ULONG64 cellCount = this->UnstructuredGridRepresentation->getCellCount();
  ULONG64 const *cumulativeFaceCountPerCell = this->UnstructuredGridRepresentation->isFaceCountOfCellsConstant()
                          ? nullptr
                          : this->UnstructuredGridRepresentation->getCumulativeFaceCountPerCell(); // This pointer is owned and managed by
                                                                                                   // FESAPI

  std::unique_ptr< unsigned char[] > cellFaceNormalOutwardlyDirected( new unsigned char[cumulativeFaceCountPerCell == nullptr
                                               ? cellCount * this->UnstructuredGridRepresentation->getConstantFaceCountOfCells()
                                               : cumulativeFaceCountPerCell[cellCount - 1]] );

  this->UnstructuredGridRepresentation->getCellFaceIsRightHanded( cellFaceNormalOutwardlyDirected.get());

  for( ULONG64 cellIndex = 0; cellIndex < cellCount; ++cellIndex )
  {
    bool isOptimizedCell = false;


    const ULONG64 localFaceCount = this->UnstructuredGridRepresentation->getFaceCountOfCell( cellIndex );

    if( localFaceCount == 4 )
    { // VTK_TETRA
      cellVtkTetra( vtk_unstructuredGrid, this->UnstructuredGridRepresentation, cumulativeFaceCountPerCell, cellFaceNormalOutwardlyDirected.get(), cellIndex );
      isOptimizedCell = true;
    }
    else if( localFaceCount == 5 )
    { // VTK_WEDGE or VTK_PYRAMID
      cellVtkWedgeOrPyramid( vtk_unstructuredGrid, this->UnstructuredGridRepresentation, cumulativeFaceCountPerCell, cellFaceNormalOutwardlyDirected.get(), cellIndex );
      isOptimizedCell = true;
    }
    else if( localFaceCount == 6 )
    { // VTK_HEXAHEDRON
      isOptimizedCell = cellVtkHexahedron( vtk_unstructuredGrid, this->UnstructuredGridRepresentation, cumulativeFaceCountPerCell, cellFaceNormalOutwardlyDirected.get(), cellIndex );
    }
    else if( localFaceCount == 7 )
    { // VTK_PENTAGONAL_PRISM
      isOptimizedCell = cellVtkPentagonalPrism( vtk_unstructuredGrid, this->UnstructuredGridRepresentation, cellIndex );
    }
    else if( localFaceCount == 8 )
    { // VTK_HEXAGONAL_PRISM
      isOptimizedCell = cellVtkHexagonalPrism( vtk_unstructuredGrid, this->UnstructuredGridRepresentation, cellIndex );
    }

    if( !isOptimizedCell )
    {
      vtkNew< vtkIdList > idList;

      // For polyhedron cell, a special ptIds input format is required : (numCellFaces, numFace0Pts, id1, id2, id3, numFace1Pts, id1, id2,
      // id3, ...)
      idList->InsertNextId( localFaceCount );
      for( ULONG64 localFaceIndex = 0; localFaceIndex < localFaceCount; ++localFaceIndex )
      {
        const unsigned int localNodeCount = this->UnstructuredGridRepresentation->getNodeCountOfFaceOfCell( cellIndex, localFaceIndex );
        idList->InsertNextId( localNodeCount );
        ULONG64 const *nodeIndices = this->UnstructuredGridRepresentation->getNodeIndicesOfFaceOfCell( cellIndex, localFaceIndex );
        for( unsigned int i = 0; i < localNodeCount; ++i )
        {
          idList->InsertNextId( nodeIndices[i] );
        }
      }

      vtk_unstructuredGrid->InsertNextCell( VTK_POLYHEDRON, idList );
    }
  }

  this->UnstructuredGridRepresentation->unloadGeometry();
}


//----------------------------------------------------------------------------
void cellVtkTetra( vtkSmartPointer< vtkUnstructuredGrid > vtk_unstructuredGrid,
                   const RESQML2_NS::UnstructuredGridRepresentation *unstructuredGridRep,
                   ULONG64 const *cumulativeFaceCountPerCell,
                   unsigned char const *cellFaceNormalOutwardlyDirected,
                   ULONG64 cellIndex )
{
  unsigned int nodes[4];

  // Face 0
  ULONG64 const *nodeIndices = unstructuredGridRep->getNodeIndicesOfFaceOfCell( cellIndex, 0 );
  size_t cellFaceIndex = unstructuredGridRep->isFaceCountOfCellsConstant() || cellIndex == 0
                 ? cellIndex * 4
                 : cumulativeFaceCountPerCell[cellIndex - 1];
  if( cellFaceNormalOutwardlyDirected[cellFaceIndex] == 0 )
  { // The RESQML orientation of face 0 honors the VTK orientation of face 0 i.e. the face 0 normal defined using a right hand rule is
    // inwardly directed.
    nodes[0] = nodeIndices[0];
    nodes[1] = nodeIndices[1];
    nodes[2] = nodeIndices[2];
  }
  else
  { // The RESQML orientation of face 0 does not honor the VTK orientation of face 0
    nodes[0] = nodeIndices[2];
    nodes[1] = nodeIndices[1];
    nodes[2] = nodeIndices[0];
  }

  // Face 1
  nodeIndices = unstructuredGridRep->getNodeIndicesOfFaceOfCell( cellIndex, 1 );

  for( size_t index = 0; index < 3; ++index )
  {
    if( std::find( nodes, nodes + 3, nodeIndices[index] ) == nodes + 3 )
    {
      nodes[3] = nodeIndices[index];
      break;
    }
  }

  vtkSmartPointer< vtkTetra > tetra = vtkSmartPointer< vtkTetra >::New();
  for( vtkIdType pointId = 0; pointId < 4; ++pointId )
  {
    tetra->GetPointIds()->SetId( pointId, nodes[pointId] );
  }
  vtk_unstructuredGrid->InsertNextCell( tetra->GetCellType(), tetra->GetPointIds());
}

//----------------------------------------------------------------------------
void cellVtkWedgeOrPyramid( vtkSmartPointer< vtkUnstructuredGrid > vtk_unstructuredGrid,
                            const RESQML2_NS::UnstructuredGridRepresentation *unstructuredGridRep,
                            ULONG64 const *cumulativeFaceCountPerCell, unsigned char const *cellFaceNormalOutwardlyDirected,
                            ULONG64 cellIndex )
{
  // The global index of the first face of the polyhedron in the cellFaceNormalOutwardlyDirected array
  const size_t globalFirstFaceIndex = unstructuredGridRep->isFaceCountOfCellsConstant() || cellIndex == 0
    ? cellIndex * 5
    : cumulativeFaceCountPerCell[cellIndex - 1];

  std::vector< unsigned int > localFaceIndexWith4Nodes;
  for( unsigned int localFaceIndex = 0; localFaceIndex < 5; ++localFaceIndex )
  {
    if( unstructuredGridRep->getNodeCountOfFaceOfCell( cellIndex, localFaceIndex ) == 4 )
    {
      localFaceIndexWith4Nodes.push_back( localFaceIndex );
    }
  }
  if( localFaceIndexWith4Nodes.size() == 3 )
  { // VTK_WEDGE
    std::array< uint64_t, 6 > nodes;
    // Set the triangle base of the wedge
    unsigned int triangleIndex = 0;
    for(; triangleIndex < 5; ++triangleIndex )
    {
      const unsigned int localNodeCount = unstructuredGridRep->getNodeCountOfFaceOfCell( cellIndex, triangleIndex );
      if( localNodeCount == 3 )
      {
        uint64_t const *nodeIndices = unstructuredGridRep->getNodeIndicesOfFaceOfCell( cellIndex, triangleIndex );
        if( cellFaceNormalOutwardlyDirected[globalFirstFaceIndex + triangleIndex] == 0 )
        {
          for( size_t i = 0; i < 3; ++i )
          {
            nodes[i] = nodeIndices[2 - i];
          }
        }
        else
        {
          // The RESQML orientation of face 0 honors the VTK orientation of face 0 i.e. the face 0 normal defined using a right hand rule is
          // outwardly directed.
          for( size_t i = 0; i < 3; ++i )
          {
            nodes[i] = nodeIndices[i];
          }
        }
        ++triangleIndex;
        break;
      }
    }
    // Find the index of the vertex at the opposite triangle regarding the triangle base
    for( unsigned int localFaceIndex = 0; localFaceIndex < 5; ++localFaceIndex )
    {
      const unsigned int localNodeCount = unstructuredGridRep->getNodeCountOfFaceOfCell( cellIndex, localFaceIndex );
      if( localNodeCount == 4 )
      {
        uint64_t const *nodeIndices = unstructuredGridRep->getNodeIndicesOfFaceOfCell( cellIndex, localFaceIndex );
        if( nodeIndices[0] == nodes[0] )
        {
          nodes[3] = nodeIndices[1] == nodes[1] || nodeIndices[1] == nodes[2]
            ? nodeIndices[3] : nodeIndices[1];
          break;
        }
        else if( nodeIndices[1] == nodes[0] )
        {
          nodes[3] = nodeIndices[2] == nodes[1] || nodeIndices[2] == nodes[2]
            ? nodeIndices[0] : nodeIndices[2];
          break;
        }
        else if( nodeIndices[2] == nodes[0] )
        {
          nodes[3] = nodeIndices[3] == nodes[1] || nodeIndices[3] == nodes[2]
            ? nodeIndices[1] : nodeIndices[3];
          break;
        }
        else if( nodeIndices[3] == nodes[0] )
        {
          nodes[3] = nodeIndices[0] == nodes[1] || nodeIndices[0] == nodes[2]
            ? nodeIndices[2] : nodeIndices[0];
          break;
        }
      }
    }
    // Set the other triangle of the wedge
    for(; triangleIndex < 5; ++triangleIndex )
    {
      const unsigned int localNodeCount = unstructuredGridRep->getNodeCountOfFaceOfCell( cellIndex, triangleIndex );
      if( localNodeCount == 3 )
      {
        uint64_t const *nodeIndices = unstructuredGridRep->getNodeIndicesOfFaceOfCell( cellIndex, triangleIndex );
        if( nodeIndices[0] == nodes[3] )
        {
          if( cellFaceNormalOutwardlyDirected[globalFirstFaceIndex + triangleIndex] == 0 )
          {
            nodes[4] = nodeIndices[1];
            nodes[5] = nodeIndices[2];
          }
          else
          {
            nodes[4] = nodeIndices[2];
            nodes[5] = nodeIndices[1];
          }
        }
        else if( nodeIndices[1] == nodes[3] )
        {
          if( cellFaceNormalOutwardlyDirected[globalFirstFaceIndex + triangleIndex] == 0 )
          {
            nodes[4] = nodeIndices[2];
            nodes[5] = nodeIndices[0];
          }
          else
          {
            nodes[4] = nodeIndices[0];
            nodes[5] = nodeIndices[2];
          }
        }
        else if( nodeIndices[2] == nodes[3] )
        {
          if( cellFaceNormalOutwardlyDirected[globalFirstFaceIndex + triangleIndex] == 0 )
          {
            nodes[4] = nodeIndices[0];
            nodes[5] = nodeIndices[1];
          }
          else
          {
            nodes[4] = nodeIndices[1];
            nodes[5] = nodeIndices[0];
          }
        }
        break;
      }
    }

    vtkSmartPointer< vtkWedge > wedge = vtkSmartPointer< vtkWedge >::New();
    for( int nodesIndex = 0; nodesIndex < 6; ++nodesIndex )
    {
      wedge->GetPointIds()->SetId( nodesIndex, nodes[nodesIndex] );
    }
    vtk_unstructuredGrid->InsertNextCell( wedge->GetCellType(), wedge->GetPointIds());
  }
  else if( localFaceIndexWith4Nodes.size() == 1 )
  { // VTK_PYRAMID
    ULONG64 nodes[5];

    ULONG64 const *nodeIndices = unstructuredGridRep->getNodeIndicesOfFaceOfCell( cellIndex, localFaceIndexWith4Nodes[0] );
    size_t cellFaceIndex = (unstructuredGridRep->isFaceCountOfCellsConstant() || cellIndex == 0
      ? cellIndex * 5
      : cumulativeFaceCountPerCell[cellIndex - 1]) +
                           localFaceIndexWith4Nodes[0];
    if( cellFaceNormalOutwardlyDirected[cellFaceIndex] == 0 )
    { // The RESQML orientation of the face honors the VTK orientation of face 0 i.e. the face 0 normal defined using a right hand rule is
      // inwardly directed.
      nodes[0] = nodeIndices[0];
      nodes[1] = nodeIndices[1];
      nodes[2] = nodeIndices[2];
      nodes[3] = nodeIndices[3];
    }
    else
    { // The RESQML orientation of the face does not honor the VTK orientation of face 0
      nodes[0] = nodeIndices[3];
      nodes[1] = nodeIndices[2];
      nodes[2] = nodeIndices[1];
      nodes[3] = nodeIndices[0];
    }

    // Face with 3 points
    nodeIndices = unstructuredGridRep->getNodeIndicesOfFaceOfCell( cellIndex, localFaceIndexWith4Nodes[0] == 0 ? 1 : 0 );

    for( size_t index = 0; index < 3; ++index )
    {
      if( std::find( nodes, nodes + 4, nodeIndices[index] ) == nodes + 4 )
      {
        nodes[4] = nodeIndices[index];
        break;
      }
    }

    vtkSmartPointer< vtkPyramid > pyramid = vtkSmartPointer< vtkPyramid >::New();
    for( int nodesIndex = 0; nodesIndex < 5; ++nodesIndex )
    {
      pyramid->GetPointIds()->SetId( nodesIndex, nodes[nodesIndex] );
    }
    vtk_unstructuredGrid->InsertNextCell( pyramid->GetCellType(), pyramid->GetPointIds());
  }
  else
  {
    throw std::invalid_argument( "The cell index " + std::to_string( cellIndex ) + " is malformed : 5 faces but not a pyramid, not a wedge." );
  }
}

//----------------------------------------------------------------------------
bool cellVtkHexahedron( vtkSmartPointer< vtkUnstructuredGrid > vtk_unstructuredGrid,
                        const RESQML2_NS::UnstructuredGridRepresentation *unstructuredGridRep,
                        ULONG64 const *cumulativeFaceCountPerCell,
                        unsigned char const *cellFaceNormalOutwardlyDirected,
                        ULONG64 cellIndex )
{
  for( ULONG64 localFaceIndex = 0; localFaceIndex < 6; ++localFaceIndex )
  {
    if( unstructuredGridRep->getNodeCountOfFaceOfCell( cellIndex, localFaceIndex ) != 4 )
    {
      return false;
    }
  }

  ULONG64 nodes[8];

  ULONG64 const *nodeIndices = unstructuredGridRep->getNodeIndicesOfFaceOfCell( cellIndex, 0 );
  const size_t cellFaceIndex = unstructuredGridRep->isFaceCountOfCellsConstant() || cellIndex == 0
                   ? cellIndex * 6
                   : cumulativeFaceCountPerCell[cellIndex - 1];
  if( cellFaceNormalOutwardlyDirected[cellFaceIndex] == 0 )
  { // The RESQML orientation of the face honors the VTK orientation of face 0 i.e. the face 0 normal defined using a right hand rule is
    // inwardly directed.
    nodes[0] = nodeIndices[0];
    nodes[1] = nodeIndices[1];
    nodes[2] = nodeIndices[2];
    nodes[3] = nodeIndices[3];
  }
  else
  { // The RESQML orientation of the face does not honor the VTK orientation of face 0
    nodes[0] = nodeIndices[3];
    nodes[1] = nodeIndices[2];
    nodes[2] = nodeIndices[1];
    nodes[3] = nodeIndices[0];
  }

  // Find the opposite neighbors of the nodes already got
  bool alreadyTreated[4] = {false, false, false, false};
  for( unsigned int localFaceIndex = 1; localFaceIndex < 6; ++localFaceIndex )
  {
    nodeIndices = unstructuredGridRep->getNodeIndicesOfFaceOfCell( cellIndex, localFaceIndex );
    for( size_t index = 0; index < 4; ++index )
    {                                 // Loop on face nodes
      ULONG64 *itr = std::find( nodes, nodes + 4, nodeIndices[index] ); // Locate a node on face 0
      if( itr != nodes + 4 )
      {
        // A top neighbor node can be found
        const size_t topNeigborIdx = std::distance( nodes, itr );
        if( !alreadyTreated[topNeigborIdx] )
        {
          const size_t previousIndex = index == 0 ? 3 : index - 1;
          nodes[topNeigborIdx + 4] = std::find( nodes, nodes + 4, nodeIndices[previousIndex] ) != nodes + 4 // If previous index is also in
                                                                                                            // face 0
                           ? nodeIndices[index == 3 ? 0 : index + 1]            // Put next index
                           : nodeIndices[previousIndex];                  // Put previous index
          alreadyTreated[topNeigborIdx] = true;
        }
      }
    }
    if( localFaceIndex > 2 && std::find( alreadyTreated, alreadyTreated + 4, false ) == alreadyTreated + 4 )
    {
      // All top neighbor nodes have been found. No need to continue
      // A minimum of four faces is necessary in order to find all top neighbor nodes.
      break;
    }
  }

  vtkSmartPointer< vtkHexahedron > hexahedron = vtkSmartPointer< vtkHexahedron >::New();
  for( int nodesIndex = 0; nodesIndex < 8; ++nodesIndex )
  {
    hexahedron->GetPointIds()->SetId( nodesIndex, nodes[nodesIndex] );
  }
  vtk_unstructuredGrid->InsertNextCell( hexahedron->GetCellType(), hexahedron->GetPointIds());
  return true;
}

//----------------------------------------------------------------------------
bool cellVtkPentagonalPrism( vtkSmartPointer< vtkUnstructuredGrid > vtk_unstructuredGrid,
                             const RESQML2_NS::UnstructuredGridRepresentation *unstructuredGridRep,
                             ULONG64 cellIndex )
{
  unsigned int faceTo5Nodes = 0;
  ULONG64 nodes[10];
  for( ULONG64 localFaceIndex = 0; localFaceIndex < 7; ++localFaceIndex )
  {
    const unsigned int localNodeCount = unstructuredGridRep->getNodeCountOfFaceOfCell( cellIndex, localFaceIndex );
    if( localNodeCount == 5 )
    {
      ULONG64 const *nodeIndices = unstructuredGridRep->getNodeIndicesOfFaceOfCell( cellIndex, localFaceIndex );
      for( unsigned int i = 0; i < localNodeCount; ++i )
      {
        nodes[faceTo5Nodes * 5 + i] = nodeIndices[i];
      }
      ++faceTo5Nodes;
    }
  }
  if( faceTo5Nodes == 2 )
  {
    vtkSmartPointer< vtkPentagonalPrism > pentagonalPrism = vtkSmartPointer< vtkPentagonalPrism >::New();
    for( int nodesIndex = 0; nodesIndex < 10; ++nodesIndex )
    {
      pentagonalPrism->GetPointIds()->SetId( nodesIndex, nodes[nodesIndex] );
    }
    vtk_unstructuredGrid->InsertNextCell( pentagonalPrism->GetCellType(), pentagonalPrism->GetPointIds());
    pentagonalPrism = nullptr;
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
bool cellVtkHexagonalPrism( vtkSmartPointer< vtkUnstructuredGrid > vtk_unstructuredGrid,
                            const RESQML2_NS::UnstructuredGridRepresentation *unstructuredGridRep,
                            ULONG64 cellIndex )
{
  unsigned int faceTo6Nodes = 0;
  ULONG64 nodes[12];
  for( ULONG64 localFaceIndex = 0; localFaceIndex < 8; ++localFaceIndex )
  {
    const unsigned int localNodeCount = unstructuredGridRep->getNodeCountOfFaceOfCell( cellIndex, localFaceIndex );
    if( localNodeCount == 6 )
    {
      ULONG64 const *nodeIndices = unstructuredGridRep->getNodeIndicesOfFaceOfCell( cellIndex, localFaceIndex );
      for( unsigned int i = 0; i < localNodeCount; ++i )
      {
        nodes[faceTo6Nodes * 6 + i] = nodeIndices[i];
      }
      ++faceTo6Nodes;
    }
  }
  if( faceTo6Nodes == 2 )
  {
    vtkSmartPointer< vtkHexagonalPrism > hexagonalPrism = vtkSmartPointer< vtkHexagonalPrism >::New();
    for( int nodesIndex = 0; nodesIndex < 12; ++nodesIndex )
    {
      hexagonalPrism->GetPointIds()->SetId( nodesIndex, nodes[nodesIndex] );
    }
    vtk_unstructuredGrid->InsertNextCell( hexagonalPrism->GetCellType(), hexagonalPrism->GetPointIds());
    hexagonalPrism = nullptr;
    return true;
  }
  return false;
}
