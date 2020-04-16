#include "structToVtk.h"
#include "vtkToStruct.h"

#include <vtkAppendFilter.h>

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    bool mergePoints = false;
    
    const std::string syntax = "outStruct = vtkAppendFilter({inStruct1, inStruct2, ...}, (mergePoints))";
    if(nrhs < 1)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 1)
        mergePoints = mxGetScalar(prhs[1]);
    if(nrhs > 2)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());

    ///// Convert MATLAB structs into vtkPointSets /////
    
    std::vector<vtkPointSet*> pointSets;
    
    const mwSize* dims = mxGetDimensions(prhs[0]);
    for(mwIndex i=0; i<dims[1]; i++)
    {
        mxArray* cellElement = mxGetCell(prhs[0], i);
        vtkPolyData* polyData = vtkPolyData::New();
        vtkUnstructuredGrid* unstructuredGrid = vtkUnstructuredGrid::New();
        vtkPointSet* pointSet = structToVtk(cellElement, polyData, unstructuredGrid, false);
        pointSets.push_back(pointSet);
        if(pointSet==polyData)
            unstructuredGrid->Delete();
        else
            polyData->Delete();
    }
    
    ///// Apply vtkAppendFilter /////
    
    vtkSmartPointer<vtkAppendFilter> appendFilter = vtkSmartPointer<vtkAppendFilter>::New();
    for(auto const& pointSet : pointSets)
        appendFilter->AddInputData(pointSet);
    appendFilter->SetMergePoints(mergePoints);
    appendFilter->Update();
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(appendFilter->GetOutput(), false);
    
    ///// Delete vtkObjects /////
    
    for(auto const& pointSet : pointSets)
        pointSet->Delete();
}
