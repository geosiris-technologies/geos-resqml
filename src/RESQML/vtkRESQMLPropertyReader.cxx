
#include "vtkRESQMLPropertyReader.h"

#include "fesapi/resqml2/CategoricalProperty.h"
#include "fesapi/resqml2/ContinuousProperty.h"
#include "fesapi/resqml2/DiscreteProperty.h"

#include <vtkUnstructuredGrid.h>
#include <vtkCellData.h>
#include <vtkSmartPointer.h>

#include <vtkObjectFactory.h>
#include <vtkNew.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkDataObject.h>

#include <vtkPointData.h>

#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkLongArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkIntArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkShortArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkCharArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkAppendFilter.h>

vtkStandardNewMacro( vtkRESQMLPropertyReader )

vtkRESQMLPropertyReader::vtkRESQMLPropertyReader()
{
  this->SetNumberOfInputPorts( 1 );
  this->SetNumberOfOutputPorts( 1 );
}

vtkRESQMLPropertyReader::~vtkRESQMLPropertyReader()
{}

void vtkRESQMLPropertyReader::PrintSelf( ostream & os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );

  os << indent << "Property type: " << this->ValuesProperty->getXmlTag()  << "\n";
}

//--------------------------------------------------------------------
int vtkRESQMLPropertyReader::RequestData(
  vtkInformation * vtkNotUsed( request ),
  vtkInformationVector * * inputVector,
  vtkInformationVector * outputVector )
{
  // get the info objects
  vtkDataSet * input = vtkDataSet::GetData( inputVector[0] );
  vtkDataSet * output = vtkDataSet::GetData( outputVector );

  // Copy all the input geometry and data to the output.
  output->CopyStructure( input );
  output->GetPointData()->PassData( input->GetPointData());
  output->GetCellData()->PassData( input->GetCellData());

  int res = this->ReadProperty( output->GetCellData());

  return res;
}


int vtkRESQMLPropertyReader::ReadProperty( vtkCellData * outDS )
{
  const gsoap_eml2_3::resqml22__IndexableElement element = this->ValuesProperty->getAttachmentKind();

  if( element != gsoap_eml2_3::resqml22__IndexableElement::cells )
  {
    vtkErrorMacro( "Property indexable element must be points or cells." );
    return 0;
  }

  std::string typeProperty = this->ValuesProperty->getXmlTag();
  if( typeProperty == RESQML2_NS::ContinuousProperty::XML_TAG )
  {
    ReadContinuousProperty( outDS );
  }
  else if( typeProperty == RESQML2_NS::DiscreteProperty::XML_TAG ||
           typeProperty == RESQML2_NS::CategoricalProperty::XML_TAG )
  {
    ReadDiscreteOrCategoricalProperty( outDS );
  }
  else
  {
    vtkErrorMacro( "property not supported...  (hdfDatatypeEnum)\n" );
    return 0;
  }

  return 1;
}

int vtkRESQMLPropertyReader::ReadContinuousProperty( vtkCellData * outDS )
{
  const unsigned int elementCountPerValue = this->ValuesProperty->getElementCountPerValue();
  const unsigned int totalHDFElementcount = this->ValuesProperty->getValuesCountOfPatch( 0 );

  double * valueIndices = new double[totalHDFElementcount]; // deleted by VTK cellData vtkSmartPointer

  this->ValuesProperty->getDoubleValuesOfPatch( 0, valueIndices );

  //TODO find a better way to handle NaN data
  for( unsigned int x = 0; x < totalHDFElementcount; ++x )
  {
    if( std::isnan( valueIndices[x] ))
    {
      valueIndices[x] = 0.00000001;
    }
  }

  vtkNew< vtkDoubleArray > cellData;
  cellData->SetArray( valueIndices, totalHDFElementcount, 0, vtkAbstractArray::VTK_DATA_ARRAY_DELETE );
  cellData->SetName( this->Name.c_str());
  cellData->SetNumberOfComponents( elementCountPerValue );

  outDS->AddArray( cellData );

  return 1;
}

int vtkRESQMLPropertyReader::ReadDiscreteOrCategoricalProperty( vtkCellData * outDS )
{
  const unsigned int elementCountPerValue = this->ValuesProperty->getElementCountPerValue();
  const unsigned int totalHDFElementcount = this->ValuesProperty->getValuesCountOfPatch( 0 );

  int * valueIndices = new int[totalHDFElementcount]; // deleted by VTK cellData vtkSmartPointer

  this->ValuesProperty->getIntValuesOfPatch( 0, valueIndices );

  //TODO handle NaN ?

  vtkNew< vtkIntArray > cellData;
  cellData->SetArray( valueIndices, totalHDFElementcount, 0, vtkAbstractArray::VTK_DATA_ARRAY_DELETE );
  cellData->SetName( this->Name.c_str());
  cellData->SetNumberOfComponents( elementCountPerValue );

  outDS->AddArray( cellData );

  return 1;
}
