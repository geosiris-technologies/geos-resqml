#ifndef vtkCellSet_h
#define vtkCellSet_h

#include <vtkObject.h>

#include <vtkMappedUnstructuredGrid.h> // For mapped unstructured grid wrapper

#include <string> // For std::string

class vtkGenericCell;
class vtkDataSet;
class vtkIdTypeArray;

/**
 * This class allows raw data arrays of cell data returned by the fesapi library
 * to be used directly in VTK without repacking the data into the vtkUnstructuredGrid
 * memory layout. Use the vtkCellSetstoCellData to read fesapi cellData
 * data into this structure.
 */
class vtkCellSetImpl : public vtkObject
{
public:
  /// @cond DO_NOT_DOCUMENT
  vtkTypeMacro( vtkCellSetImpl, vtkObject )
  /// @endcond

  /**
   * @brief Methods invoked by print to print information about the object including superclasses.
   * @param[in] os An output stream
   * @param[in] indent The indentation of the information to print
   */
  void PrintSelf( ostream & os,
                  vtkIndent indent ) override;

  /**
   * @brief Method to create a new instance
   * @return a new instance
   */
  static vtkCellSetImpl *New();

  /**
   * @brief Set the Data Set object
   */
  void SetDataSet( vtkDataSet * );

  /**
   * @brief Set the Cell Ids for the cell Set
   */
  void SetCellIds( vtkIdTypeArray * );

  /**
   * @brief Get the Cell Ids object
   * @return an array of cell ids
   */
  vtkIdTypeArray *GetCellIds();

  /**
   * \defgroup API for vtkMappedUnstructuredGrid's implementation.
   */
  /**@{*/
  /// @cond DO_NOT_DOCUMENT
  vtkIdType GetNumberOfCells();
  int GetCellType( vtkIdType cellId );
  void GetCellPoints( vtkIdType cellId, vtkIdList *ptIds );
  void GetFaceStream( vtkIdType cellId, vtkIdList *ptIds );
  void GetPointCells( vtkIdType ptId, vtkIdList *cellIds );
  int GetMaxCellSize();
  void GetIdsOfCellsOfType( int type, vtkIdTypeArray *array );
  int IsHomogeneous();
  /// @endcond
  /**@}*/

  /// @cond DO_NOT_DOCUMENT
  // This container is read only
  // These methods do nothing but print a warning.
  void Allocate( vtkIdType numCells, int extSize = 1000 );
  vtkIdType InsertNextCell( int type, vtkIdList *ptIds );
  vtkIdType InsertNextCell( int type, vtkIdType npts, const vtkIdType *ptIds );
  vtkIdType InsertNextCell( int type, vtkIdType npts, const vtkIdType *ptIds,
                            vtkIdType nfaces, const vtkIdType *faces );
  void ReplaceCell( vtkIdType cellId, int npts, const vtkIdType *pts );
  /// @endcond

protected:
  /// @cond DO_NOT_DOCUMENT
  vtkCellSetImpl();
  ~vtkCellSetImpl();
  /// @endcond

private:
  /// @cond DO_NOT_DOCUMENT
  vtkCellSetImpl( const vtkCellSetImpl & ); // Not implemented.
  void operator=( const vtkCellSetImpl & ); // Not implemented.
  /// @endcond

  /// Pointer to a dataset
  vtkDataSet *DataSet;

  /// Array of cell ids
  vtkIdTypeArray *CellIds;
};

// TODO Should use this macro but compilation issue do to aggressive warnings level
// vtkMakeMappedUnstructuredGrid(vtkCellSet, vtkCellSetImpl);

/**
 * This class is the public class to use of vtkCellSet
 */
class vtkCellSet : public vtkMappedUnstructuredGrid< vtkCellSetImpl >
{
public:
  /// @cond DO_NOT_DOCUMENT
  vtkTypeMacro(
    vtkCellSet,
    vtkMappedUnstructuredGrid< vtkCellSetImpl > )
  static vtkCellSet *New();
  /// @endcond

protected:
  /// @cond DO_NOT_DOCUMENT
  vtkCellSet()
  {
    vtkCellSetImpl *i = vtkCellSetImpl::New();
    this->SetImplementation( i );
    i->Delete();
  }
  ~vtkCellSet() override {}
  /// @endcond

private:
  /// @cond DO_NOT_DOCUMENT
  vtkCellSet( const vtkCellSet & );
  void operator=( const vtkCellSet & );
  /// @endcond
};

#endif // vtkCellSet_h
