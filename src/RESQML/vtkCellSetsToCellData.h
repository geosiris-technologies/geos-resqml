
#ifndef vtkCellSetsToCellData_h
#define vtkCellSetsToCellData_h

#include <vtkDataObjectAlgorithm.h>

class vtkDataSetCollection;

/**
 * This class allows to convert vtkCellSets to cell data arrays.
 */
class vtkCellSetsToCellData : public vtkDataObjectAlgorithm
{
public:
  /// @cond DO_NOT_DOCUMENT
  vtkTypeMacro( vtkCellSetsToCellData, vtkDataObjectAlgorithm )
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
  static vtkCellSetsToCellData *New();

  /// @cond DO_NOT_DOCUMENT
  vtkSetMacro( Name, std::string & );
  vtkGetMacro( Name, std::string );
  vtkSetMacro( Representation, vtkDataSet * );
  /// @endcond

protected:
  /// @cond DO_NOT_DOCUMENT
  vtkCellSetsToCellData();
  ~vtkCellSetsToCellData();

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
  vtkCellSetsToCellData( const vtkCellSetsToCellData & );  // Not implemented.
  void operator=( const vtkCellSetsToCellData & );  // Not implemented.
  /// @endcond

  /// Name of cell attribute for this representation
  std::string Name{"attribute"};

  /// Pointer to the representation
  vtkDataSet * Representation{nullptr};
};

#endif
