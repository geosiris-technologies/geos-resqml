#ifndef vtkRESQMLPropertyReader_h
#define vtkRESQMLPropertyReader_h

#include "fesapi/resqml2/AbstractValuesProperty.h"

// #include "fesapi/nsDefinitions.h"

#include <vtkDataSetAlgorithm.h>
#include <vtkCellData.h>

namespace RESQML2_NS
{
class AbstractValuesProperty;
}

/**
 * This class allows to read RESQML Properties returned by the fesapi library
 * directly in VTK cell data.
 */
class vtkRESQMLPropertyReader : public vtkDataSetAlgorithm
{
public:
  /// @cond DO_NOT_DOCUMENT
  vtkTypeMacro( vtkRESQMLPropertyReader, vtkDataSetAlgorithm )
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
  static vtkRESQMLPropertyReader *New();

  /// @cond DO_NOT_DOCUMENT
  vtkSetMacro( ValuesProperty,
               RESQML2_NS::AbstractValuesProperty * );
  vtkSetMacro( Name, std::string & );
  vtkGetMacro( Name, std::string );
  /// @endcond

protected:
  /// @cond DO_NOT_DOCUMENT
  vtkRESQMLPropertyReader();
  ~vtkRESQMLPropertyReader() override;

  int RequestData( vtkInformation * request,
                   vtkInformationVector * * inputVector,
                   vtkInformationVector * outputVector ) override;
  /// @endcond

  /**
   * @brief Read a property from an AbstractValueProperty
   * @param[in,out] outDS
   * @return 1 if read was successful, 0 if no read was performed
   */
  int ReadProperty( vtkCellData * outDS );

  /**
   * @brief Read a continuous property
   * @param[in,out] outDS the cell data to fill
   * @return 1 if read was successful, 0 if no read was performed
   */
  int ReadContinuousProperty( vtkCellData * outDS );

  /**
   * @brief Read a discrete or categorical property
   * @param[in, out] outDS  the cell data to fill
   * @return 1 if read was successful, 0 if no read was performed
   */
  int ReadDiscreteOrCategoricalProperty( vtkCellData * outDS );

private:
  /// @cond DO_NOT_DOCUMENT
  vtkRESQMLPropertyReader( const vtkRESQMLPropertyReader & );  // Not implemented.
  void operator=( const vtkRESQMLPropertyReader & );  // Not implemented.
  /// @endcond

  /// Pointer to the property in fesapi
  RESQML2_NS::AbstractValuesProperty *ValuesProperty{nullptr};

  /// Name of the attribute property in vtk
  std::string Name{"attribute"};
};

#endif
