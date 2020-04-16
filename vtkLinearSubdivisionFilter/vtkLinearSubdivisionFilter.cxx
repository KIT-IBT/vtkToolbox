#include "structToVtk.h"
#include "vtkToStruct.h"

#include <vtkLinearSubdivisionFilter.h>

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    int numberOfSubdivisions = 1;
    
    const std::string syntax = "outStruct = vtkLinearSubdivisionFilter(inStruct, numberOfSubdivisions)";
    if(nrhs < 1)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 1)
        numberOfSubdivisions = mxGetScalar(prhs[1]);
    if(nrhs > 2)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());

    ///// Convert MATLAB struct into vtkPointSet /////
    
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet = structToVtk(prhs[0], polyData, unstructuredGrid, false);
    
    ///// Apply vtkLinearSubdivisionFilter /////
    
    if(pointSet->GetDataObjectType() != VTK_POLY_DATA)
        mexErrMsgTxt("vtkLinearSubdivisionFilter requires poly data as input. Incompatible cell(s) found.");
    
    vtkSmartPointer<vtkLinearSubdivisionFilter> linearSubdivisionFilter = vtkSmartPointer<vtkLinearSubdivisionFilter>::New();
    linearSubdivisionFilter->SetInputData(pointSet);
    linearSubdivisionFilter->SetNumberOfSubdivisions(numberOfSubdivisions);
    linearSubdivisionFilter->Update();
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(linearSubdivisionFilter->GetOutput(), false);
}
