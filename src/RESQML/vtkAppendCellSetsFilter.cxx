#include "vtkAppendCellSetsFilter.h"

#include <vtkObjectFactory.h>
#include <vtkNew.h>
#include <vtkDemandDrivenPipeline.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkAlgorithm.h>
#include <vtkDataSetCollection.h>
#include "vtkCellSet.h"


vtkStandardNewMacro( vtkAppendCellSetsFilter )


vtkAppendCellSetsFilter::vtkAppendCellSetsFilter()
{ }

vtkAppendCellSetsFilter::~vtkAppendCellSetsFilter()
{ }

void vtkAppendCellSetsFilter::PrintSelf( ostream & os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );

  os << indent << "vtkAppendCellSetsFilter " << "\n";
}

int vtkAppendCellSetsFilter::RequestDataObject(
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

int vtkAppendCellSetsFilter::RequestData(
  vtkInformation * vtkNotUsed( request ),
  vtkInformationVector * * inputVector,
  vtkInformationVector * outputVector )
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkDataObject* input = inInfo->Get(vtkDataObject::DATA_OBJECT());

  // get the info objects
  vtkInformation * outInfo = outputVector->GetInformationObject( 0 );

  // get the input and output
  vtkDataObject *output = vtkDataObject::SafeDownCast(
    outInfo->Get( vtkDataObject::DATA_OBJECT()));

  //TODO check if arr exists if output exists ?
  output->ShallowCopy(input);


  vtkSmartPointer< vtkDataSetCollection > inputs;
  inputs.TakeReference( this->GetNonEmptyInputs( inputVector ));

  vtkCollectionSimpleIterator iter;
  inputs->InitTraversal( iter );
  // vtkDataSet * dataSet = nullptr;

  // while((dataSet = inputs->GetNextDataSet( iter )))
  // {

    // vtkCellSet *a = vtkCellSet::SafeDownCast( dataSet );
    // vtkIdType dataSetNumCells = a->GetNumberOfCells();

    // for( const vtkIdType val : vtk::DataArrayValueRange( a->GetImplementation()->GetCellIds()))
    // {
    //   vtkNew<vtkIdList> cellPointIds;
    //   a->GetCellPoints(cellId, cellPointIds);
    //   output->InsertNextCell(dataSet->GetCellType(cellId), cellPointIds);      
    // }
  // }

//   output->GetCellData()->AddArray( arr );

//   this->Representation->GetCellData()->AddArray( arr );

  return 1;
}

// vtkIdType vtkAppendCellSetsFilter::GetTotalNumberOfCellIds( vtkDataSetCollection * collection )
// {
//   vtkIdType count( 0 );

//   vtkCollectionSimpleIterator iter;
//   collection->InitTraversal( iter );
//   vtkDataSet * dataSet = nullptr;
//   while((dataSet = collection->GetNextDataSet( iter )))
//   {
//     vtkCellSet *a = vtkCellSet::SafeDownCast( dataSet );
//     count += a->GetNumberOfCells();
//   }

//   // int nb_of_inputs = this->GetTotalNumberOfInputConnections();
//   // for(int r = 0 ; r < nb_of_inputs ; ++r)
//   // {
//   //   vtkInformation *inInfo = collection [r]->GetInformationObject(0);
//   //   vtkDataSet *input = vtkDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

//   //   count += input->GetNumberOfCells();
//   // }

//   return count;
// }

vtkDataSetCollection * vtkAppendCellSetsFilter::GetNonEmptyInputs( vtkInformationVector * * inputVector )
{
  vtkDataSetCollection * collection = vtkDataSetCollection::New();
  int numInputs = inputVector[1]->GetNumberOfInformationObjects();
  for( int inputIndex = 0; inputIndex < numInputs; ++inputIndex )
  {
    vtkInformation * inInfo = inputVector[1]->GetInformationObject( inputIndex );
    vtkDataSet * dataSet = nullptr;
    if( inInfo )
    {
      dataSet = vtkDataSet::SafeDownCast( inInfo->Get( vtkDataObject::DATA_OBJECT()));
    }
    if( dataSet != nullptr )
    {
    //   if( dataSet->GetNumberOfPoints() <= 0 && dataSet->GetNumberOfCells() <= 0 )
    //   {
    //     continue; // no input, just skip
    //   }
      collection->AddItem( dataSet );
    }
  }

  return collection;
}

int vtkAppendCellSetsFilter::RequestUpdateExtent( vtkInformation * vtkNotUsed( request ),
                                                vtkInformationVector * * inputVector, vtkInformationVector * vtkNotUsed( outputVector ))
{
  int numInputConnections = this->GetNumberOfInputConnections( 1 );

  // Let downstream request a subset of connection 0, for connections >= 1
  // send their WHOLE_EXTENT as UPDATE_EXTENT.
  for( int idx = 1; idx < numInputConnections; ++idx )
  {
    vtkInformation * inputInfo = inputVector[1]->GetInformationObject( idx );
    if( inputInfo->Has( vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()))
    {
      int ext[6];
      inputInfo->Get( vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext );
      inputInfo->Set( vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), ext, 6 );
    }
  }

  return 1;
}

int vtkAppendCellSetsFilter::FillInputPortInformation( int, vtkInformation * info )
{
  info->Set( vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkCellSet", 1);
  info->Set( vtkAlgorithm::INPUT_IS_REPEATABLE(), 1 );
  return 1;
}
