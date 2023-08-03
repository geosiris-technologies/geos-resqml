#ifndef vtkAppendCellSetsFilter_h
#define vtkAppendCellSetsFilter_h

#include <vtkDataObjectAlgorithm.h>

class vtkDataSetCollection;

/**
 * This class allows to append vtkCellSets to an unstructured grid.
 */
class vtkAppendCellSetsFilter : public vtkDataObjectAlgorithm
{
public:
  /// @cond DO_NOT_DOCUMENT
  vtkTypeMacro( vtkAppendCellSetsFilter, vtkDataObjectAlgorithm )
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
  static vtkAppendCellSetsFilter *New();

protected:
  /// @cond DO_NOT_DOCUMENT
  vtkAppendCellSetsFilter();
  ~vtkAppendCellSetsFilter();

  int RequestDataObject(
    vtkInformation *request, vtkInformationVector * *inputVector, vtkInformationVector *outputVector ) override;

  int RequestData( vtkInformation * request,
                   vtkInformationVector * * inputVector,
                   vtkInformationVector * outputVector ) override;

  int RequestUpdateExtent( vtkInformation* vtkNotUsed( request ), vtkInformationVector * * inputVector, vtkInformationVector* vtkNotUsed( outputVector )) override;

  int FillInputPortInformation( int, vtkInformation * info ) override;
  /// @endcond

  /**
   * @brief Get the Total Number Of Cell Ids in a collection
   * @param[in] collection a collection
   * @return number of cell ids
   */
  vtkIdType GetTotalNumberOfCellIds( vtkDataSetCollection * collection );

  /**
   * @brief Get the Non Empty Inputs object
   * @param[in] inputVector of information
   * @return collection of cell data
   */
  vtkDataSetCollection * GetNonEmptyInputs( vtkInformationVector * * inputVector );


private:
  /// @cond DO_NOT_DOCUMENT
  vtkAppendCellSetsFilter( const vtkAppendCellSetsFilter & );  // Not implemented.
  void operator=( const vtkAppendCellSetsFilter & );  // Not implemented.
  /// @endcond
};

#endif