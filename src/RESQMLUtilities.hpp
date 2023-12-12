/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2020 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2020 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2020 Total, S.A
 * Copyright (c) 2019-     GEOS Contributors
 * All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

/**
 * @file RESQMLUtilities.hpp
 */

#ifndef GEOS_EXTERNALCOMPONENTS_RESQML_RESQMLUTILITIES_HPP
#define GEOS_EXTERNALCOMPONENTS_RESQML_RESQMLUTILITIES_HPP

#include "common/DataTypes.hpp"

#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkExplicitStructuredGrid.h>
#include <vtkDataSet.h>

#include "fesapi/resqml2/UnstructuredGridRepresentation.h"
#include "fesapi/resqml2/AbstractIjkGridRepresentation.h"

namespace geos
{

/**
 * @brief Load a RESQML Grid
 * 
 * @param[in] rep The RESQML grid ad an AbstractObject
 * @return the loaded dataset
 * 
 * @todo Handle hyperslab
 * @details Handles UnstructuredGridRepresentation and IjkGridRepresentation
 */
vtkSmartPointer< vtkDataSet >
loadGridRepresentation(COMMON_NS::AbstractObject *rep );

/**
 * @brief Load an IjkGridRepresentation
 * 
 * @param[in] rep 
 * @return The loaded dataset
 */
vtkSmartPointer< vtkDataSet >
loadIjkGridRepresentation(RESQML2_NS::AbstractIjkGridRepresentation* rep);


/**
 * @brief Load a RESQML UnstructuredGriRepresentation in a vtkUnstructuredGrid
 *
 * @param[in] grid The RESQML UnstructuredGriRepresentation
 * @return The loaded dataset
 */
vtkSmartPointer< vtkDataSet >
loadUnstructuredGridRepresentation( RESQML2_NS::UnstructuredGridRepresentation *grid );

/**
 * @brief Load a Property in an existing dataset
 *
 * @param[in] dataset The existing dataset
 * @param[in] valuesProperty The RESQML Property
 * @param[in] fieldNameInGEOS The name of property in GEOS
 * @return The dataset with the loaded property
 */
vtkSmartPointer< vtkDataSet >
loadProperty( vtkSmartPointer< vtkDataSet > dataset, RESQML2_NS::AbstractValuesProperty *valuesProperty, string fieldNameInGEOS );

/**
 * @brief Create a cell array of regions with an array of RESQML SubRepresentations
 *
 * @param dataset The existing dataset
 * @param regions The array of RESQML SubRepresentations
 * @param attributeName The name of the vtk cell array
 * @return The dataset with the loaded regions
 */
vtkSmartPointer< vtkDataSet >
createRegions( vtkSmartPointer< vtkDataSet > dataset, std::vector< RESQML2_NS::SubRepresentation * > regions, string attributeName );

/**
 * @brief Create as many surfaces in a dataset as RESQML SubRepresentations
 *
 * @param dataset The existing dataset
 * @param surfaces The array of RESQML SubRepresentations
 * @param regionAttributeName The name of the region array name
 * @return The dataset with the loaded surfaces
 */
vtkUnstructuredGrid *
createSurfaces( vtkSmartPointer< vtkDataSet > dataset, std::vector< std::pair<integer, RESQML2_NS::SubRepresentation *> >& surfaces, string regionAttributeName );


} // namespace geos

#endif /* GEOS_EXTERNALCOMPONENTS_RESQML_RESQMLUTILITIES_HPP */