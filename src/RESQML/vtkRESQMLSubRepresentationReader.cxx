#include "vtkRESQMLSubRepresentationReader.h"

#include <vtkObjectFactory.h>
#include <vtkNew.h>
#include <vtkDemandDrivenPipeline.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkAlgorithm.h>

#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkTriangle.h>
#include <vtkQuad.h>

#include "vtkCellSet.h"

#include "fesapi/resqml2/UnstructuredGridRepresentation.h"

vtkStandardNewMacro( vtkRESQMLSubRepresentationReader )

vtkRESQMLSubRepresentationReader::vtkRESQMLSubRepresentationReader()
{
  this->SetNumberOfInputPorts( 1 );
  this->SetNumberOfOutputPorts( 1 );
}

vtkRESQMLSubRepresentationReader::~vtkRESQMLSubRepresentationReader()
{}

void vtkRESQMLSubRepresentationReader::PrintSelf( ostream & os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );

  os << indent << "Uuid: "
     << this->SubRepresentation->getUuid() << "\n";
}

int vtkRESQMLSubRepresentationReader::RequestDataObject(
  vtkInformation *vtkNotUsed( request ),
  vtkInformationVector * *vtkNotUsed( inputVector ),
  vtkInformationVector *outputVector )
{
  vtkInformation * info = outputVector->GetInformationObject( 0 );
  vtkCellSet *output = vtkCellSet::SafeDownCast(
    info->Get( vtkDataObject::DATA_OBJECT()));

  if( !output )
  {
    vtkCellSet * newOutput = vtkCellSet::New();
    info->Set( vtkDataObject::DATA_OBJECT(), newOutput );
    newOutput->Delete();
  }

  return 1;
}

int vtkRESQMLSubRepresentationReader::RequestData(
  vtkInformation * vtkNotUsed( request ),
  vtkInformationVector * * inputVector,
  vtkInformationVector * outputVector )
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject( 0 );
  vtkInformation *outInfo = outputVector->GetInformationObject( 0 );

  // get the input and output
  vtkPointSet *input = vtkPointSet::SafeDownCast( inInfo->Get( vtkDataObject::DATA_OBJECT()));
  vtkCellSet *output = vtkCellSet::SafeDownCast( outInfo->Get( vtkDataObject::DATA_OBJECT()));

  vtkPointSet * inputCopy = input->NewInstance();
  inputCopy->ShallowCopy( input );
  output->GetImplementation()->SetDataSet( inputCopy );
  inputCopy->Delete();

  output->SetPoints(inputCopy->GetPoints());
  output->GetPointData()->PassData(inputCopy->GetPointData());
  output->GetCellData()->PassData(inputCopy->GetCellData());

	if (this->SubRepresentation->getElementKindOfPatch(0, 0) == gsoap_eml2_3::resqml22__IndexableElement::cells)
	{
    vtkIdType nb_cells( 0 );
    for( unsigned int i = 0; i < this->SubRepresentation->getPatchCount(); ++i )
    {
      nb_cells += this->SubRepresentation->getElementCountOfPatch( i );
    }

    // modify the Copy Tuple flag for GlobalIds
    output->GetCellData()->CopyGlobalIdsOn();

    uint64_t * elementIndices = new uint64_t[nb_cells];
    uint64_t * ptr_indices_start = elementIndices;
    for( unsigned int i = 0; i < this->SubRepresentation->getPatchCount(); ++i )
    {
      uint64_t elementCountOfPatch = this->SubRepresentation->getElementCountOfPatch( i );
      this->SubRepresentation->getElementIndicesOfPatch( i, 0, ptr_indices_start );
      ptr_indices_start += elementCountOfPatch;
    }

    //reinterpret uint64 (aka 64bits unsigned long) into vtkIdType (aka 64 bits long long -> signed)
    vtkIdType * elements = reinterpret_cast< vtkIdType * >(elementIndices);

    vtkNew< vtkIdTypeArray > ids;
    ids->SetNumberOfComponents( 1 );
    ids->SetArray( elements, nb_cells, vtkAbstractArray::VTK_DATA_ARRAY_DELETE );

    output->GetImplementation()->SetCellIds( ids.GetPointer());
  
  }
  else if (this->SubRepresentation->getElementKindOfPatch(0, 0) == gsoap_eml2_3::resqml22__IndexableElement::faces)
	{				
    vtkNew<vtkPolyData> polydata;

		// FACES
 		auto *supportingGrid = dynamic_cast<RESQML2_NS::UnstructuredGridRepresentation *>(this->SubRepresentation->getSupportingRepresentation(0));

		const uint64_t gridFaceCount = supportingGrid->getFaceCount();
		std::unique_ptr<uint64_t[]> nodeCountOfFaces(new uint64_t[gridFaceCount]);
		supportingGrid->getCumulativeNodeCountPerFace(nodeCountOfFaces.get());
				
  	std::unique_ptr<uint64_t[]> nodeIndices(new uint64_t[nodeCountOfFaces[gridFaceCount-1]]);
		supportingGrid->getNodeIndicesOfFaces(nodeIndices.get());

		const uint64_t subFaceCount = this->SubRepresentation->getElementCountOfPatch(0);
		std::unique_ptr<uint64_t[]> elementIndices(new uint64_t[subFaceCount]);
		this->SubRepresentation->getElementIndicesOfPatch(0, 0, elementIndices.get());

		vtkNew<vtkCellArray> polys;
		polys->AllocateEstimate(subFaceCount, 4);
		for (uint64_t subFaceIndex =0; subFaceIndex < subFaceCount; ++subFaceIndex)
		{
			uint64_t faceIndex = elementIndices[subFaceIndex];
			auto first_indiceValue = faceIndex == 0 ? 0 : nodeCountOfFaces[faceIndex - 1];
			uint64_t nodeCount_OfFaceIndex = nodeCountOfFaces[faceIndex] - first_indiceValue;

			vtkSmartPointer<vtkCell> cell;
			if (nodeCount_OfFaceIndex == 3)
			{
				cell = vtkSmartPointer<vtkTriangle>::New();
			}
			else if (nodeCount_OfFaceIndex == 4)
			{
				cell = vtkSmartPointer<vtkQuad>::New();
			}
			else
			{
				cell = vtkSmartPointer<vtkPolygon>::New();
				cell->GetPointIds()->SetNumberOfIds(nodeCount_OfFaceIndex);
			}

			for (uint64_t nodeIndex = 0; nodeIndex < nodeCount_OfFaceIndex; ++nodeIndex, ++first_indiceValue)
			{
				cell->GetPointIds()->SetId(nodeIndex, nodeIndices[first_indiceValue]);
			}

      polys->InsertNextCell(cell);  
		}
    
    polydata->SetPolys(polys);
    output->GetImplementation()->SetDataSet(polydata);

    //vtkIdType * elements = reinterpret_cast< vtkIdType * >(elementIndices.get());

    vtkNew< vtkIdTypeArray > ids;
    ids->SetNumberOfComponents( 1 );
    ids->Allocate(subFaceCount);
    // ids->SetArray( elements, subFaceCount, vtkAbstractArray::VTK_DATA_ARRAY_DELETE );

    for( unsigned int i = 0; i < subFaceCount; ++i )
    {
      ids->InsertNextValue(i);// std::cout << elementIndices[i] << "\t";
    }
    std::cout << std::endl;

    output->GetImplementation()->SetCellIds( ids.GetPointer());
  }


  return 1;
}

// vtkNew<vtkIdTypeArray> ids;
//   ids->SetNumberOfTuples(nb_cells);
//   vtkIdType global_i=0;
//   for(unsigned int i = 0 ; i < this->SubRepresentation->getPatchCount() ; ++i)
//   {
//     uint64_t elementCountOfPatch = this->SubRepresentation->getElementCountOfPatch(i);
//     std::unique_ptr<uint64_t[]> elementIndices(new uint64_t[elementCountOfPatch]);
//     this->SubRepresentation->getElementIndicesOfPatch(i, 0, elementIndices.get());

//     for(size_t e = 0 ;  e < elementCountOfPatch ; ++e)
//     {
//       ids->SetValue(global_i, vtkIdType(elementIndices[e]));
//       ++global_i;
//     }
//   }
