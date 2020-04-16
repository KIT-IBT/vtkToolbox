#include "structToVtk.h"
#include "vtkToStruct.h"

#include <vtkFeatureEdges.h>

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    bool boundaryEdgesOn = true;
    bool featureEdgesOn = true;
    bool nonManifoldEdgesOn = true;
    bool manifoldEdgesOn = false;
    bool coloringOn = true;
    double featureAngle = 30.0;
    
    const std::string syntax = "outStruct = vtkFeatureEdges(inStruct, (boundaryEdgesOn), (featureEdgesOn), (nonManifoldEdgesOn), (manifoldEdgesOn), (coloringOn), (featureAngle))";
    if(nrhs < 1)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 1)
        boundaryEdgesOn = mxGetScalar(prhs[1]);
    if(nrhs > 2)
        featureEdgesOn = mxGetScalar(prhs[2]);
    if(nrhs > 3)
        nonManifoldEdgesOn = mxGetScalar(prhs[3]);
    if(nrhs > 4)
        manifoldEdgesOn = mxGetScalar(prhs[4]);
    if(nrhs > 5)
        coloringOn = mxGetScalar(prhs[5]);
    if(nrhs > 6)
        featureAngle = mxGetScalar(prhs[6]);
    if(nrhs > 7)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());

    ///// Convert MATLAB struct into vtkPointSet /////
    
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet = structToVtk(prhs[0], polyData, unstructuredGrid, false);
    
    ///// Apply operations to pointSet /////
    
    if(pointSet->GetDataObjectType() != VTK_POLY_DATA)
        mexErrMsgTxt("vtkLinearSubdivisionFilter requires poly data as input. Incompatible cell(s) found.");
    
    vtkSmartPointer<vtkFeatureEdges> featureEdges = vtkSmartPointer<vtkFeatureEdges>::New();
    featureEdges->SetInputData(pointSet);
    featureEdges->SetBoundaryEdges(boundaryEdgesOn);
    featureEdges->SetFeatureEdges(featureEdgesOn);
    featureEdges->SetNonManifoldEdges(nonManifoldEdgesOn);
    featureEdges->SetManifoldEdges(manifoldEdgesOn);
    featureEdges->SetColoring(coloringOn);
    featureEdges->SetFeatureAngle(featureAngle);
    featureEdges->Update();
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(featureEdges->GetOutput(), false);
}
