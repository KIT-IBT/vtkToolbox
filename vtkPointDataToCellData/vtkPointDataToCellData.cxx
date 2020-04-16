#include "structToVtk.h"
#include "vtkToStruct.h"

#include <vtkPointDataToCellData.h>

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    bool passPointData = false;
    
    const std::string syntax = "outStruct = vtkPointDataToCellData(inStruct, (passPointData))";
    if(nrhs < 1)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 1)
        passPointData = mxGetScalar(prhs[1]);
    if(nrhs > 2)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());

    ///// Convert MATLAB struct into vtkPointSet /////
    
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet = structToVtk(prhs[0], polyData, unstructuredGrid, false);
    
    ///// Apply vtkPointDataToCellData /////
    
    vtkSmartPointer<vtkPointDataToCellData> pointDataToCellData = vtkSmartPointer<vtkPointDataToCellData>::New();
    pointDataToCellData->SetInputData(pointSet);
    pointDataToCellData->SetPassPointData(passPointData);
    pointDataToCellData->Update();
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    if(pointSet->GetDataObjectType() == VTK_POLY_DATA)
        plhs[0] = vtkToStruct(pointDataToCellData->GetPolyDataOutput(), false);
    else
        plhs[0] = vtkToStruct(pointDataToCellData->GetUnstructuredGridOutput(), false);
}
