/**
 * @class   vtkRESQMLUnstructuredGridReader
 * @brief   extract unstructured grid geometry of a RESQML Unstructured Grid
 *
 * vtkRESQMLUnstructuredGridReader is a general-purpose filter to
 * extract geometry (and associated data) from an unstructured grid
 * dataset.
 *
 */
#ifndef vtkRESQMLUnstructuredGridReader_h
#define vtkRESQMLUnstructuredGridReader_h

#include "fesapi/resqml2/AbstractLocal3dCrs.h"
#include "fesapi/resqml2/UnstructuredGridRepresentation.h"

#include <vtkUnstructuredGridAlgorithm.h>

class vtkUnstructuredGrid;


class vtkRESQMLUnstructuredGridReader : public vtkUnstructuredGridAlgorithm
{
public:
  /// @cond DO_NOT_DOCUMENT
  vtkTypeMacro( vtkRESQMLUnstructuredGridReader, vtkUnstructuredGridAlgorithm )
  /// @endcond

  /**
   * @brief Methods invoked by print to print information about the object including superclasses.
   * @param[in] os An output stream
   * @param[in] indent The indentation of the information to print
   */
  void PrintSelf( ostream & os, vtkIndent indent ) override;

  /**
   * @brief Method to create a new instance
   * @return a new instance
   */
  static vtkRESQMLUnstructuredGridReader *New();

  /// @cond DO_NOT_DOCUMENT
  vtkSetMacro( UnstructuredGridRepresentation,
               RESQML2_NS::UnstructuredGridRepresentation * );
  /// @endcond

protected:
  /// @cond DO_NOT_DOCUMENT
  vtkRESQMLUnstructuredGridReader();
  ~vtkRESQMLUnstructuredGridReader();

  // int FillOutputPortInformation( int port, vtkInformation* info ) override ;
  int RequestData( vtkInformation *request,
                   vtkInformationVector * *inputVector,
                   vtkInformationVector *outputVector ) override;

  /// @endcond

private:
  /// @cond DO_NOT_DOCUMENT
  vtkRESQMLUnstructuredGridReader( const vtkRESQMLUnstructuredGridReader & );  // Not implemented.
  void operator=( const vtkRESQMLUnstructuredGridReader & );  // Not implemented.
  /// @endcond

  /// @brief Build an unstructured grid from the data provided by fesapi
  /// @param[in, out] vtk_unstructuredGrid an empty grid to build
  void buildUnstructuredGrid( vtkSmartPointer< vtkUnstructuredGrid > vtk_unstructuredGrid );

  /// A pointer to an unstructured grid
  RESQML2_NS::UnstructuredGridRepresentation *UnstructuredGridRepresentation{nullptr};
};

/**
 * @brief Free function that generates a vtkTetrahedra from a cellIndex in an UnstructuredGridRepresentation.
 *
 * @param[in,out] vtk_unstructuredGrid
 * @param[in] unstructuredGridRep
 * @param[in] cumulativeFaceCountPerCell
 * @param[in] cellFaceNormalOutwardlyDirected
 * @param[in] cellIndex
 */
void cellVtkTetra( vtkSmartPointer< vtkUnstructuredGrid > vtk_unstructuredGrid,
                   const RESQML2_NS::UnstructuredGridRepresentation *unstructuredGridRep,
                   ULONG64 const *cumulativeFaceCountPerCell,
                   unsigned char const *cellFaceNormalOutwardlyDirected,
                   ULONG64 cellIndex );

/**
 * @brief
 *
 * @param[in,out] vtk_unstructuredGrid
 * @param[in] unstructuredGridRep
 * @param[in] cumulativeFaceCountPerCell
 * @param[in] cellFaceNormalOutwardlyDirected
 * @param[in] cellIndex
 */
void cellVtkWedgeOrPyramid( vtkSmartPointer< vtkUnstructuredGrid > vtk_unstructuredGrid,
                            const RESQML2_NS::UnstructuredGridRepresentation *unstructuredGridRep,
                            ULONG64 const *cumulativeFaceCountPerCell,
                            unsigned char const *cellFaceNormalOutwardlyDirected,
                            ULONG64 cellIndex );

/**
 * @brief Free function that generates a vtkHexahedron from a cellIndex in an UnstructuredGridRepresentation.
 *
 * @param[in,out] vtk_unstructuredGrid
 * @param[in] unstructuredGridRep
 * @param[in] cumulativeFaceCountPerCell
 * @param[in] cellFaceNormalOutwardlyDirected
 * @param[in] cellIndex
 * @return true
 * @return false
 */
bool cellVtkHexahedron( vtkSmartPointer< vtkUnstructuredGrid > vtk_unstructuredGrid,
                        const RESQML2_NS::UnstructuredGridRepresentation *unstructuredGridRep,
                        ULONG64 const *cumulativeFaceCountPerCell,
                        unsigned char const *cellFaceNormalOutwardlyDirected,
                        ULONG64 cellIndex );

/**
 * @brief Free function that generates a VtkPentagonalPrism from a cellIndex in an UnstructuredGridRepresentation.
 *
 * @param[in,out] vtk_unstructuredGrid
 * @param[in] unstructuredGridRep
 * @param[in] cellIndex
 * @return true
 * @return false
 */
bool cellVtkPentagonalPrism( vtkSmartPointer< vtkUnstructuredGrid > vtk_unstructuredGrid,
                             const RESQML2_NS::UnstructuredGridRepresentation *unstructuredGridRep,
                             ULONG64 cellIndex );

/**
 * @brief Free function that generates a VtkHexagonalPrism from a cellIndex in an UnstructuredGridRepresentation.
 *
 * @param vtk_unstructuredGrid
 * @param unstructuredGridRep
 * @param cellIndex
 * @return true
 * @return false
 */
bool cellVtkHexagonalPrism( vtkSmartPointer< vtkUnstructuredGrid > vtk_unstructuredGrid,
                            const RESQML2_NS::UnstructuredGridRepresentation *unstructuredGridRep,
                            ULONG64 cellIndex );

#endif
