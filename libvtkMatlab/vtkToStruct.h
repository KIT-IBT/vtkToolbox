/******************************************************************
 vtkToStruct
 Converts a vtkPointSet (either vtkPolyData or vtkUnstructuredGrid)
 into a MATLAB struct.
 ------------------------------------------------------------------
 Author: Steffen Schuler
         Institute of Biomedical Engineering
         Karlsruhe Institute of Technology
         www.ibt.kit.edu
 
 Created: June 2017
 ******************************************************************/

#ifndef vtkToStruct_h
#define vtkToStruct_h

#include "common.h"

// general
#include <vtkSmartPointer.h>

// data sets
#include <vtkPointSet.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>

// data set attributes
#include <vtkPointData.h>
#include <vtkCellData.h>

// filters
#include <vtkTriangleFilter.h>

// mex
#include "mex.h"
#include "matrix.h"

mxArray* vtkToStruct(vtkPointSet* inPointSet, bool verbose);

#endif /* vtkToStruct_h */
