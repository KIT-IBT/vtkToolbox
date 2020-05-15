#include "structToVtk.h"
#include "vtkToStruct.h"

#include <vtkLinearExtrusionFilter.h>
#include <vtkTriangleFilter.h>

#include <cmath>

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    const std::string syntax = "outStruct = vtkLinearExtrusionFilter(inStruct, vector, scaleFactor)";
    if(nrhs < 3)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 3)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());

    ///// Convert MATLAB struct into vtkPointSet /////
    
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet = structToVtk(prhs[0], polyData, unstructuredGrid, false);
    
    ///// Apply linearExtrusionFilter filter /////
    
    vtkSmartPointer<vtkLinearExtrusionFilter> linearExtrusionFilter = vtkSmartPointer<vtkLinearExtrusionFilter>::New();
    linearExtrusionFilter->SetInputData(pointSet);
    linearExtrusionFilter->SetExtrusionTypeToVectorExtrusion();
    
    if(mxGetNumberOfElements(prhs[1]) != 3)
        mexErrMsgTxt("numel(vector) must be 3.");
    double* vector = mxGetPr(prhs[1]);
    linearExtrusionFilter->SetVector(vector);

    if(mxGetNumberOfElements(prhs[2]) != 1)
        mexErrMsgTxt("numel(scaleFactor) must be 1.");
    double scaleFactor = mxGetScalar(prhs[2]);
    linearExtrusionFilter->SetScaleFactor(scaleFactor);
    
    linearExtrusionFilter->Update();
    
    vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
    triangleFilter->SetInputData(linearExtrusionFilter->GetOutput());
    triangleFilter->Update();
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(triangleFilter->GetOutput(), false);
}
