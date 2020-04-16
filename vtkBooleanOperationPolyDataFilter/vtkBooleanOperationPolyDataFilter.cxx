#include "structToVtk.h"
#include "vtkToStruct.h"

#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkCleanPolyData.h>

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    int operation = vtkBooleanOperationPolyDataFilter::VTK_UNION;
    
    const std::string syntax = "outStruct = vtkBooleanOperationPolyDataFilter(inStruct1, inStruct2, (operation))";
    if(nrhs > 2)
    {
        char* operationString = mxArrayToString(prhs[2]);
        if(strcmp(operationString, "intersection") == 0)
            operation = vtkBooleanOperationPolyDataFilter::VTK_INTERSECTION;
        else if(strcmp(operationString, "difference") == 0)
            operation = vtkBooleanOperationPolyDataFilter::VTK_DIFFERENCE;
        else if(strcmp(operationString, "union") != 0)
            mexWarnMsgTxt(("Unknown operation '" + std::string(operationString) + "'. Using default 'union' instead.").c_str());
    }
    if(nrhs < 2)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 3)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());

    ///// Convert MATLAB struct into vtkPointSet /////
    
    vtkSmartPointer<vtkPolyData> polyData1 = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid1 = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet1 = structToVtk(prhs[0], polyData1, unstructuredGrid1, false);
    
    vtkSmartPointer<vtkPolyData> polyData2 = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid2 = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet2 = structToVtk(prhs[1], polyData2, unstructuredGrid2, false);
    
    ///// Apply vtkBooleanOperationPolyDataFilter /////
    
    if(pointSet1->GetDataObjectType() != VTK_POLY_DATA)
        mexErrMsgTxt("vtkBooleanOperationPolyDataFilter requires poly data as input. Incompatible cell(s) found in input 1.");
    if(pointSet2->GetDataObjectType() != VTK_POLY_DATA)
        mexErrMsgTxt("vtkBooleanOperationPolyDataFilter requires poly data as input. Incompatible cell(s) found in input 2.");
    
    vtkSmartPointer<vtkBooleanOperationPolyDataFilter> booleanFilter = vtkSmartPointer<vtkBooleanOperationPolyDataFilter>::New();
    booleanFilter->SetInputData(0, pointSet1);
    booleanFilter->SetInputData(1, pointSet2);
    booleanFilter->SetOperation(operation);
    booleanFilter->Update();
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(booleanFilter->GetOutput(), false);
}
