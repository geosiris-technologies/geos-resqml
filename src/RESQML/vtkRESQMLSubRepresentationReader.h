#ifndef vtkRESQMLSubRepresentationReader_h
#define vtkRESQMLSubRepresentationReader_h

#include "fesapi/resqml2/SubRepresentation.h"

#include <vtkUnstructuredGridBaseAlgorithm.h>

/**
 * This class allows to read RESQML subrepresentation returned by the fesapi library
 * directly in VTK cell data of an unstructured grid.
 */
class vtkRESQMLSubRepresentationReader : public vtkUnstructuredGridBaseAlgorithm
{
public:
  /// @cond DO_NOT_DOCUMENT
  vtkTypeMacro( vtkRESQMLSubRepresentationReader, vtkUnstructuredGridBaseAlgorithm )
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
  static vtkRESQMLSubRepresentationReader *New();

  /// @cond DO_NOT_DOCUMENT
  vtkSetMacro( SubRepresentation,
               RESQML2_NS::SubRepresentation * );
  /// @endcond
protected:
  /// @cond DO_NOT_DOCUMENT
  vtkRESQMLSubRepresentationReader();
  ~vtkRESQMLSubRepresentationReader();

  int RequestDataObject(
    vtkInformation *request, vtkInformationVector * *inputVector, vtkInformationVector *outputVector ) override;

  int RequestData( vtkInformation * request,
                   vtkInformationVector * * inputVector,
                   vtkInformationVector * outputVector ) override;
  /// @endcond

private:
  /// @cond DO_NOT_DOCUMENT
  vtkRESQMLSubRepresentationReader( const vtkRESQMLSubRepresentationReader & );  // Not implemented.
  void operator=( const vtkRESQMLSubRepresentationReader & );  // Not implemented.
  /// @endcond

  /// Pointer to a RESQML subrepresentation
  RESQML2_NS::SubRepresentation *SubRepresentation{nullptr};
};

#endif
