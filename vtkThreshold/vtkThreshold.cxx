#include "structToVtk.h"
#include "vtkToStruct.h"

#include <vtkThreshold.h>

#include <cmath>

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    const std::string syntax = "outStruct = vtkThreshold(inStruct, cellsOrPoints, arrayName, thresholds)";
    if(nrhs < 4)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 4)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());

    ///// Convert MATLAB struct into vtkPointSet /////
    
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet = structToVtk(prhs[0], polyData, unstructuredGrid, false);
    
    ///// Apply threshold filter /////
    
    vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
    threshold->SetInputData(pointSet);
    
    char* cellsOrPoints = mxArrayToString(prhs[1]);
    char* arrayName = mxArrayToString(prhs[2]);
    if(!getDataArray(pointSet, cellsOrPoints, arrayName))
        mexErrMsgTxt(("Data array \"" + std::string(arrayName) + "\" could not be found. Also check the argument cellsOrPoints.").c_str());
    
    if(strcmp(cellsOrPoints, "cells") == 0)
        threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, arrayName);
    else if(strcmp(cellsOrPoints, "points") == 0)
        threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, arrayName);
    else
        mexErrMsgTxt("cellsOrPoints must be \"cells\" or \"points\".");
    
    double* thresholds = mxGetPr(prhs[3]);
    if(!std::isfinite(thresholds[0])) {
        threshold->SetThresholdFunction(vtkThreshold::THRESHOLD_LOWER);
        threshold->SetLowerThreshold(thresholds[1]);
    } else if(!std::isfinite(thresholds[1])) {
        threshold->SetThresholdFunction(vtkThreshold::THRESHOLD_UPPER);
        threshold->SetUpperThreshold(thresholds[0]);
    } else {
        threshold->SetThresholdFunction(vtkThreshold::THRESHOLD_BETWEEN);
        threshold->SetLowerThreshold(thresholds[0]);
        threshold->SetUpperThreshold(thresholds[1]);
    }
    threshold->Update();
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(threshold->GetOutput(), false);
}
