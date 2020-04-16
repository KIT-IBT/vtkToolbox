#include "structToVtk.h"
#include "vtkToStruct.h"

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    const std::string syntax = "outStruct = vtkTEMPLATE(inStruct)";
    if(nrhs < 1)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 1)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());

    ///// Convert MATLAB struct into vtkPointSet /////
    
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet = structToVtk(prhs[0], polyData, unstructuredGrid, false);
    
    ///// Apply operations to pointSet /////
    
//    if(pointSet->GetDataObjectType() != VTK_POLY_DATA)
//        mexErrMsgTxt("vtkLinearSubdivisionFilter requires poly data as input. Incompatible cell(s) found.");
    
//    vtkSmartPointer<vtkTEMPLATE> TEMPLATE = vtkSmartPointer<vtkTEMPLATE>::New();
//    TEMPLATE->SetInputData(pointSet);
//    TEMPLATE->Update();
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(TEMPLATE->GetOutput(), false);
}
