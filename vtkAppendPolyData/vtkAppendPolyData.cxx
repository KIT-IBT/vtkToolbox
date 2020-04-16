#include "structToVtk.h"
#include "vtkToStruct.h"

#include <vtkAppendPolyData.h>

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    bool mergePoints = false;
    
    const std::string syntax = "outStruct = vtkAppendPolyData({inStruct1, inStruct2, ...})";
    if(nrhs < 1)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 1)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());

    ///// Convert MATLAB structs into vtkPointSets /////
    
    std::vector<vtkPolyData*> polyDatas;
    
    const mwSize* dims = mxGetDimensions(prhs[0]);
    for(mwIndex i=0; i<dims[1]; i++)
    {
        mxArray* cellElement = mxGetCell(prhs[0], i);
        vtkPolyData* polyData = vtkPolyData::New();
        vtkUnstructuredGrid* unstructuredGrid = vtkUnstructuredGrid::New();
        vtkPointSet* pointSet = structToVtk(cellElement, polyData, unstructuredGrid, false);
        if(pointSet->GetDataObjectType() != VTK_POLY_DATA)
        {
            polyData->Delete();
            unstructuredGrid->Delete();
            for(auto const& polyData : polyDatas)
                polyData->Delete();
            mexErrMsgTxt(("vtkAppendPolyData requires poly data as input. Incompatible cell(s) found in input " + std::to_string(i+1) + ".").c_str());
        }
        polyDatas.push_back(polyData);
        unstructuredGrid->Delete();
    }
    
    ///// Apply vtkAppendPolyData /////
    
    vtkSmartPointer<vtkAppendPolyData> appendPolyData = vtkSmartPointer<vtkAppendPolyData>::New();
    for(auto const& polyData : polyDatas)
        appendPolyData->AddInputData(polyData);
    appendPolyData->Update();
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(appendPolyData->GetOutput(), false);
    
    ///// Delete vtkObjects /////
    
    for(auto const& polyData : polyDatas)
        polyData->Delete();
}
