/*=========================================================================

   Program:   Visualization Toolkit
   Module:    vtkCellSet.h

   Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
   All rights reserved.
   See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

   =========================================================================*/

#include "vtkCellSet.h"

#include "vtkCellType.h"
#include "vtkCellTypes.h"
#include "vtkGenericCell.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkDataSet.h"

#include <algorithm>

//------------------------------------------------------------------------------
vtkStandardNewMacro( vtkCellSet )
vtkStandardNewMacro( vtkCellSetImpl )

vtkCxxSetObjectMacro( vtkCellSetImpl, DataSet, vtkDataSet )
vtkCxxSetObjectMacro( vtkCellSetImpl, CellIds, vtkIdTypeArray )

//------------------------------------------------------------------------------
void vtkCellSetImpl::PrintSelf( ostream & os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//------------------------------------------------------------------------------
vtkIdTypeArray *vtkCellSetImpl::GetCellIds()
{
  return this->CellIds;
}

//------------------------------------------------------------------------------
vtkIdType vtkCellSetImpl::GetNumberOfCells()
{
  return this->CellIds->GetNumberOfTuples();
}

//------------------------------------------------------------------------------
int vtkCellSetImpl::GetCellType( vtkIdType id )
{
  return this->DataSet->GetCellType( this->CellIds->GetValue( id ));
}

//------------------------------------------------------------------------------
void vtkCellSetImpl::GetCellPoints( vtkIdType cellId,
                                    vtkIdList *ptIds )
{
  return this->DataSet->GetCellPoints( this->CellIds->GetValue( cellId ),
                                       ptIds );
}

//------------------------------------------------------------------------------
void vtkCellSetImpl::GetPointCells( vtkIdType, vtkIdList * )
{
  // TODO: Implement this.
}

void vtkCellSetImpl::GetFaceStream( vtkIdType, vtkIdList * )
{}

//------------------------------------------------------------------------------
int vtkCellSetImpl::GetMaxCellSize()
{
  return this->DataSet->GetMaxCellSize();
}

//------------------------------------------------------------------------------
void vtkCellSetImpl::GetIdsOfCellsOfType( int, vtkIdTypeArray * )
{
  // TODO: Implement this.
}

//------------------------------------------------------------------------------
int vtkCellSetImpl::IsHomogeneous()
{
  return 0;
}

//------------------------------------------------------------------------------
void vtkCellSetImpl::Allocate( vtkIdType, int )
{
  vtkErrorMacro( "Read only container." );
}

//------------------------------------------------------------------------------
vtkIdType vtkCellSetImpl::InsertNextCell( int, vtkIdList * )
{
  vtkErrorMacro( "Read only container." );
  return vtkIdType( -1 );
}

//------------------------------------------------------------------------------
vtkIdType vtkCellSetImpl::InsertNextCell( int, vtkIdType, const vtkIdType * )
{
  vtkErrorMacro( "Read only container." );
  return vtkIdType( -1 );
}

//------------------------------------------------------------------------------
vtkIdType vtkCellSetImpl::InsertNextCell(
  int, vtkIdType, const vtkIdType *, vtkIdType, const vtkIdType * )
{
  vtkErrorMacro( "Read only container." );
  return vtkIdType( -1 );
}

//------------------------------------------------------------------------------
void vtkCellSetImpl::ReplaceCell( vtkIdType, int, const vtkIdType * )
{
  vtkErrorMacro( "Read only container." );
}

//------------------------------------------------------------------------------
vtkCellSetImpl::vtkCellSetImpl()
  : DataSet( NULL ),
  CellIds( NULL )
{}

//------------------------------------------------------------------------------
vtkCellSetImpl::~vtkCellSetImpl()
{
  if( this->DataSet )
  {
    this->DataSet->Delete();
    this->DataSet = 0;
  }

  if( this->CellIds )
  {
    this->CellIds->Delete();
    this->CellIds = 0;
  }
}
