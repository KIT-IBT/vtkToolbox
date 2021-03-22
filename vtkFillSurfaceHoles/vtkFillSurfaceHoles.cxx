/************************************************************
 * This function uses Constantine Butakoff's code from
 * https://github.com/cbutakoff/tools/tree/master/FillSurfaceHoles (commit 8c2cac1),
 * which implements the algorithms in
 * P. Liepa (2003), Filling Holes in Meshes, http://dx.doi.org/10.2312/SGP/SGP03/200-206
 * G. Barequet et al. (1995), Filling gaps in the boundary of a polyhedron, https://doi.org/10.1016/0167-8396(94)00011-G
 ************************************************************/

#include "structToVtk.h"
#include "vtkToStruct.h"

#include "FillSurfaceHoles/SurfaceHoleFiller.h"

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////

    bool doSmoothing = true;
    int smoothingType = EDGE_WEIGHT_COTANGENT;
    bool verbose = false;
    
    const std::string syntax = "outStruct = vtkFillSurfaceHoles(inStruct, (method), (verbose))";
    if(nrhs < 1)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 1 && !mxIsEmpty(prhs[1]))
    {
        char* method = mxArrayToString(prhs[1]);
        if(strcmp(method, "none") == 0)
            doSmoothing = false;
        else if(strcmp(method, "edgelen") == 0)
            smoothingType = EDGE_WEIGHT_InvEdgeLength;
        else if(strcmp(method, "cot") != 0)
            mexWarnMsgTxt("Unknown method, using default \"cot\".");
    }
    if(nrhs > 2 && !mxIsEmpty(prhs[2]))
        verbose = mxGetScalar(prhs[2]);
    if(nrhs > 3)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());

    ///// Convert MATLAB struct into vtkPointSet /////
    
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet = structToVtk(prhs[0], polyData, unstructuredGrid, false);
    
    ///// Apply vtkCleanPolyData /////
    
    if(pointSet->GetDataObjectType() != VTK_POLY_DATA)
        mexErrMsgTxt("vtkCleanPolyData requires poly data as input. Incompatible cell(s) found.");
    
    SurfaceHoleFiller holeFiller;
    holeFiller.SetVerbose(verbose);
    holeFiller.SetInput(polyData);
    holeFiller.SetSmoothing(doSmoothing);
    holeFiller.SetEdgeWeightingType(smoothingType);
    holeFiller.Update();
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(holeFiller.GetOutput(), false);
}
