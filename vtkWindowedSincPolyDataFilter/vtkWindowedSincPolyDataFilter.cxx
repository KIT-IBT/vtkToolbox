#include "structToVtk.h"
#include "vtkToStruct.h"

#include <vtkWindowedSincPolyDataFilter.h>

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    int numberOfIterations = 20;
    double passBand = 0.1;
    bool featureEdgeSmoothing = false;
    double featureAngle = 45;
    double edgeAngle = 45;
    bool boundarySmoothing = false;
    
    const std::string syntax = "outStruct = vtkWindowedSincPolyDataFilter(inStruct, (numberOfIterations), (passband), (featureEdgeSmoothing), (featureAngle), (edgeAngle), (boundarySmoothing))";
    if(nrhs < 1)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 1)
        numberOfIterations = mxGetScalar(prhs[1]);
    if(nrhs > 2)
        passBand = mxGetScalar(prhs[2]);
    if(nrhs > 3)
        featureEdgeSmoothing = mxGetScalar(prhs[3]);
    if(nrhs > 4)
        featureAngle = mxGetScalar(prhs[4]);
    if(nrhs > 5)
        featureAngle = mxGetScalar(prhs[5]);
    if(nrhs > 6)
        boundarySmoothing = mxGetScalar(prhs[6]);
    if(nrhs > 7)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());

    ///// Convert MATLAB struct into vtkPointSet /////
    
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet = structToVtk(prhs[0], polyData, unstructuredGrid, false);
    
    ///// Apply vtkWindowedSincPolyDataFilter /////
    
    if(pointSet->GetDataObjectType() != VTK_POLY_DATA)
        mexErrMsgTxt("vtkWindowedSincPolyDataFilter requires poly data as input. Incompatible cell(s) found.");
    
    vtkSmartPointer<vtkWindowedSincPolyDataFilter> windowedSincPolyDataFilter = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
    windowedSincPolyDataFilter->SetInputData(pointSet);
    windowedSincPolyDataFilter->SetNumberOfIterations(numberOfIterations);
    windowedSincPolyDataFilter->SetPassBand(passBand);
    windowedSincPolyDataFilter->SetFeatureEdgeSmoothing(featureEdgeSmoothing);
    windowedSincPolyDataFilter->SetFeatureAngle(featureAngle);
    windowedSincPolyDataFilter->SetEdgeAngle(edgeAngle);
    windowedSincPolyDataFilter->SetBoundarySmoothing(boundarySmoothing);
    windowedSincPolyDataFilter->NormalizeCoordinatesOn();
    windowedSincPolyDataFilter->Update();
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(windowedSincPolyDataFilter->GetOutput(), false);
}
