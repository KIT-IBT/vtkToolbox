/*************************************************************
 structToVtk
 Converts a MATLAB struct into a vtkPointSet
 (either vtkPolyData or vtkUnstructuredGrid).
 -------------------------------------------------------------
 The struct must have the following fields:
     points:    [numPoints x numDim double/single/(u)intN], numDim <= 3
     cells:     [numCells x maxNumPointsPerCell int32]
     cellTypes: [numCells x 1 uint8] (see https://vtk.org/doc/nightly/html/vtkCellType_8h_source.html)
     pointData: [1 x 1 struct of point data arrays] (optional)
     cellData:  [1 x 1 struct of cell data arrays] (optional)
     fieldData: [1 x 1 struct of field data arrays] (optional)
 -------------------------------------------------------------
 Authors: Steffen Schuler, Nils Daub
          Institute of Biomedical Engineering
          Karlsruhe Institute of Technology
          www.ibt.kit.edu
 
 Created: November 2017
 *************************************************************/

#ifndef structToVtk_h
#define structToVtk_h

#include "common.h"

// general
#include <vtkSmartPointer.h>
#include <vtkErrorCode.h>

// data sets
#include <vtkPointSet.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>

// data set attributes
#include <vtkPointData.h>
#include <vtkCellData.h>

// mex
#include "mex.h"
#include "matrix.h"

vtkPointSet* structToVtk(const mxArray* inStruct, vtkPolyData* polyData, vtkUnstructuredGrid* unstructuredGrid, bool verbose);

#endif /* structToVtk_h */
