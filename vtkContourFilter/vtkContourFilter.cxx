#include "structToVtk.h"
#include "vtkToStruct.h"

#include <vtkContourFilter.h>

#include <cmath>

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    const std::string syntax = "outStruct = vtkContourFilter(inStruct, cellsOrPoints, arrayName, values)";
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
    
    ///// Apply vtkContourFilter /////
    
    vtkSmartPointer<vtkContourFilter> contourFilter = vtkSmartPointer<vtkContourFilter>::New();
    contourFilter->SetInputData(pointSet);
    
    char* cellsOrPoints = mxArrayToString(prhs[1]);
    char* arrayName = mxArrayToString(prhs[2]);
    if(!getDataArray(pointSet, cellsOrPoints, arrayName))
        mexErrMsgTxt(("Data array \"" + std::string(arrayName) + "\" could not be found. Also check the argument cellsOrPoints.").c_str());
    
    if(strcmp(cellsOrPoints, "cells") == 0)
        contourFilter->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, arrayName);
    else if(strcmp(cellsOrPoints, "points") == 0)
        contourFilter->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, arrayName);
    else
        mexErrMsgTxt("cellsOrPoints must be \"cells\" or \"points\".");
    
    mwSize numValues = mxGetNumberOfElements(prhs[3]);
    double* values = mxGetPr(prhs[3]);
    for(mwSize i = 0; i < numValues; i++)
    {
        if(!std::isfinite(values[i]))
            mexErrMsgTxt(("values(" + std::to_string(i) + ") is not finite.").c_str());
        contourFilter->SetValue(i, values[i]);
    }
    
    contourFilter->Update();
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(contourFilter->GetOutput(), false);
}
