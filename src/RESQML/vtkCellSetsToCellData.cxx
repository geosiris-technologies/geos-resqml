#include "vtkCellSetsToCellData.h"

#include <vtkObjectFactory.h>
#include <vtkNew.h>
#include <vtkDemandDrivenPipeline.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkAlgorithm.h>
#include <vtkCellData.h>
#include <vtkDataSetCollection.h>
#include <vtkIntArray.h>
#include <vtkDataArrayRange.h>
#include "vtkCellSet.h"

vtkStandardNewMacro( vtkCellSetsToCellData )


vtkCellSetsToCellData::vtkCellSetsToCellData()
{ }

vtkCellSetsToCellData::~vtkCellSetsToCellData()
{ }

void vtkCellSetsToCellData::PrintSelf( ostream & os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );

  os << indent << "plop " << "\n";
}

int vtkCellSetsToCellData::RequestDataObject(
  vtkInformation *vtkNotUsed( request ),
  vtkInformationVector * *vtkNotUsed( inputVector ),
  vtkInformationVector *outputVector )
{
  vtkInformation * info = outputVector->GetInformationObject( 0 );
  vtkDataObject *output = vtkDataObject::SafeDownCast(
    info->Get( vtkDataObject::DATA_OBJECT()));

  if( !output )
  {
    vtkDataObject * newOutput = vtkDataObject::New();
    info->Set( vtkDataObject::DATA_OBJECT(), newOutput );
    newOutput->Delete();
  }

  return 1;
}

int vtkCellSetsToCellData::RequestData(
  vtkInformation * vtkNotUsed( request ),
  vtkInformationVector * * inputVector,
  vtkInformationVector * outputVector )
{
  // get the info objects
  vtkInformation * outInfo = outputVector->GetInformationObject( 0 );

  // get the input and output
  vtkDataObject *output = vtkDataObject::SafeDownCast(
    outInfo->Get( vtkDataObject::DATA_OBJECT()));

  //TODO check if arr exists if output exists ?

  vtkSmartPointer< vtkDataSetCollection > inputs;
  inputs.TakeReference( this->GetNonEmptyInputs( inputVector ));

  int region_id = 0;

  vtkIntArray * arr = vtkIntArray::New();
  arr->SetName( this->Name.c_str());
  arr->SetNumberOfComponents( 1 );
  arr->SetNumberOfTuples( this->Representation->GetNumberOfCells());
  arr->FillValue( region_id );

  vtkDataArrayAccessor< vtkIntArray > attribute( arr );

  vtkCollectionSimpleIterator iter;
  inputs->InitTraversal( iter );
  vtkDataSet * dataSet = nullptr;

  while((dataSet = inputs->GetNextDataSet( iter )))
  {
    region_id += 1;

    vtkCellSet *a = vtkCellSet::SafeDownCast( dataSet );
    for( const vtkIdType val : vtk::DataArrayValueRange( a->GetImplementation()->GetCellIds()))
    {
      attribute.Set( val, 0, region_id );
    }
  }

  output->GetFieldData()->AddArray( arr );

  this->Representation->GetCellData()->AddArray( arr );

  return 1;
}

vtkIdType vtkCellSetsToCellData::GetTotalNumberOfCellIds( vtkDataSetCollection * collection )
{
  vtkIdType count( 0 );

  vtkCollectionSimpleIterator iter;
  collection->InitTraversal( iter );
  vtkDataSet * dataSet = nullptr;
  while((dataSet = collection->GetNextDataSet( iter )))
  {
    vtkCellSet *a = vtkCellSet::SafeDownCast( dataSet );
    count += a->GetNumberOfCells();
  }

  // int nb_of_inputs = this->GetTotalNumberOfInputConnections();
  // for(int r = 0 ; r < nb_of_inputs ; ++r)
  // {
  //   vtkInformation *inInfo = collection [r]->GetInformationObject(0);
  //   vtkDataSet *input = vtkDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

  //   count += input->GetNumberOfCells();
  // }

  return count;
}

vtkDataSetCollection * vtkCellSetsToCellData::GetNonEmptyInputs( vtkInformationVector * * inputVector )
{
  vtkDataSetCollection * collection = vtkDataSetCollection::New();
  int numInputs = inputVector[0]->GetNumberOfInformationObjects();
  for( int inputIndex = 0; inputIndex < numInputs; ++inputIndex )
  {
    vtkInformation * inInfo = inputVector[0]->GetInformationObject( inputIndex );
    vtkDataSet * dataSet = nullptr;
    if( inInfo )
    {
      dataSet = vtkDataSet::SafeDownCast( inInfo->Get( vtkDataObject::DATA_OBJECT()));
    }
    if( dataSet != nullptr )
    {
      if( dataSet->GetNumberOfPoints() <= 0 && dataSet->GetNumberOfCells() <= 0 )
      {
        continue; // no input, just skip
      }
      collection->AddItem( dataSet );
    }
  }

  return collection;
}

int vtkCellSetsToCellData::RequestUpdateExtent( vtkInformation * vtkNotUsed( request ),
                                                vtkInformationVector * * inputVector, vtkInformationVector * vtkNotUsed( outputVector ))
{
  int numInputConnections = this->GetNumberOfInputConnections( 0 );

  // Let downstream request a subset of connection 0, for connections >= 1
  // send their WHOLE_EXTENT as UPDATE_EXTENT.
  for( int idx = 1; idx < numInputConnections; ++idx )
  {
    vtkInformation * inputInfo = inputVector[0]->GetInformationObject( idx );
    if( inputInfo->Has( vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()))
    {
      int ext[6];
      inputInfo->Get( vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext );
      inputInfo->Set( vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), ext, 6 );
    }
  }

  return 1;
}

int vtkCellSetsToCellData::FillInputPortInformation( int, vtkInformation * info )
{
  info->Set( vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkCellSet" );
  info->Set( vtkAlgorithm::INPUT_IS_REPEATABLE(), 1 );
  return 1;
}
