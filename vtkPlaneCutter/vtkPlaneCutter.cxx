#include "structToVtk.h"
#include "vtkToStruct.h"

#include <vtkPlaneCutter.h>
#include <vtkPlane.h>

#include <cmath>

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    const std::string syntax = "outStruct = vtkPlaneCutter(inStruct, origin, normal)";
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
    
    ///// Apply vtkPlaneCutter /////

    double* origin = mxGetPr(prhs[1]);
    double* normal = mxGetPr(prhs[2]);

    vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(origin);
    plane->SetNormal(normal);
    
    vtkSmartPointer<vtkPlaneCutter> planeCutter = vtkSmartPointer<vtkPlaneCutter>::New();
    planeCutter->SetInputData(pointSet);
    planeCutter->SetPlane(plane);
    planeCutter->Update();
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(planeCutter->GetOutput(), false);
}
