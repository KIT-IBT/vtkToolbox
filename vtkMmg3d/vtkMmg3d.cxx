#include "vtkToStruct.h"
#include "structToVtk.h"

#include "mmg/mmg3d/libmmg3d.h"

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    const std::string syntax = "outStruct = vtkMmg3d(inStruct, arrayName, isoValue, hmin, hmax, hausd, logFile)";
    if(nrhs < 3)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 7)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());
    
    const mxArray* inStruct = prhs[0];
    char* arrayName = mxArrayToString(prhs[1]);
    double isoValue = mxGetScalar(prhs[2]);
    double hmin = -1, hmax = -1, hausd = -1;
    if(nrhs > 3)
        hmin = mxGetScalar(prhs[3]);
    if(nrhs > 4)
        hmax = mxGetScalar(prhs[4]);
    if(nrhs > 5)
        hausd = mxGetScalar(prhs[5]);
    char* logFile = NULL;
    if(nrhs > 6)
    {
        // Redirect stdout to log file
        logFile = mxArrayToString(prhs[6]);
        if(freopen(logFile, "w", stdout) == NULL)
            mexErrMsgTxt(("Log file '" + std::string(logFile) + "' could not be opened.").c_str());
    }
    
    ///// Process VTK struct /////
    
    mxArray* points = mxGetField(inStruct, 0, "points");
    if(points == NULL)
        mexErrMsgTxt("Field 'points' of input struct could not be read.");
    if(!mxIsClass(points, "double"))
        mexErrMsgTxt("Field 'points' of input struct must be of class double. You could try points = double(points)");
    
    mxArray* cells = mxGetField(inStruct, 0, "cells");
    if(cells == NULL)
        mexErrMsgTxt("Field 'cells' of input struct could not be read.");
    if(!mxIsClass(cells, "int32"))
        mexErrMsgTxt("Field 'cells' of input struct must be of class int32. You could try cells = int32(cells)");
    
    mxArray* cellTypes = mxGetField(inStruct, 0, "cellTypes");
    if(cellTypes == NULL)
        mexErrMsgTxt("Field 'cellTypes' of input struct could not be read.");
    if(!mxIsClass(cellTypes, "uint8"))
        mexErrMsgTxt("Field 'cellTypes' of input struct must be of class uint8. You could try cellTypes = uint8(cellTypes)");
    
    mxArray* pointDataStruct = mxGetField(inStruct, 0, "pointData");
    mxArray* solArray = mxGetField(pointDataStruct, 0, arrayName);
    if(solArray == NULL)
        mexErrMsgTxt(("Point data array '" + std::string(arrayName) + "' could not be read.").c_str());
    if(!mxIsClass(solArray, "double"))
        mexErrMsgTxt(("Field 'pointData." + std::string(arrayName) + "' of input struct must be of class double. You could try " + std::string(arrayName) + " = double(" + std::string(arrayName) + ")").c_str());
    
    ///// Check file format compatibility /////
    
    // Make sure there are only linear tetrahedra
    for(size_t i = 0; i < mxGetNumberOfElements(cellTypes); i++)
    {
        uint8_t cellType = ((uint8_t*)mxGetData(cellTypes))[i];
        if(cellType != VTK_TETRA)
            mexErrMsgTxt(("Only linear tetrahedra are supported, but cellTypes(" + std::to_string(i) + ") ~= " + std::to_string(VTK_TETRA) + ".").c_str());
    }
    
    if(mxGetN(points) != 3)
        mexErrMsgTxt("size(points,2) must be 3.");
    
    if(mxGetN(cells) != 4)
        mexErrMsgTxt("size(cells,2) must be 4.");
    
    if(mxGetNumberOfElements(cellTypes) != mxGetM(cells))
        mexErrMsgTxt("numel(cellTypes) must be equal to size(cells,1).");
    
    if(mxGetNumberOfElements(solArray) != mxGetM(points))
        mexErrMsgTxt(("numel('pointData." + std::string(arrayName) + "') must be equal to size(points,1).").c_str());
    
    ///// Convert input /////

    // process points
    int32_T numPoints = mxGetM(points);
    double* p = new double[3*numPoints];
    int32_T k = 0;
    for(int32_T i = 0; i < numPoints; i++)
    {
        for(int32_T j = 0; j < 3; j++)
        {
            p[k] = ((double*)mxGetData(points))[numPoints*j+i];
            k++;
        }
    }

    // process cells
    int32_T numCells = mxGetM(cells);
    int32_T* c = new int32_T[4*numCells];
    k = 0;
    for(int32_T i = 0; i < numCells; i++)
    {
        for(int32_T j = 0; j < 4; j++)
        {
            c[k] = ((int32_T*)mxGetData(cells))[numCells*j+i];
            k++;
        }
    }
    
    // process data array
    double* s = new double[numPoints];
    for(int32_T i = 0; i < numPoints; i++)
        s[i] = ((double*)mxGetData(solArray))[i];
    
    ///// Run MMG /////
    
    MMG5_pMesh mmgMesh = NULL;
    MMG5_pSol mmgSol = NULL;
    
    if(!MMG3D_Init_mesh(MMG5_ARG_start, MMG5_ARG_ppMesh, &mmgMesh, MMG5_ARG_ppLs, &mmgSol, MMG5_ARG_end))
        mexErrMsgTxt("MMG3D_Init_mesh failed.");
    
    if(!MMG3D_Set_meshSize(mmgMesh, numPoints, numCells, 0, 0, 0, 0))
        mexErrMsgTxt("MMG3D_Set_meshSize failed.");
    
    int* pointRefs = new int[numPoints] ();
    if(!MMG3D_Set_vertices(mmgMesh, p, pointRefs))
        mexErrMsgTxt("MMG3D_Set_vertices failed.");
    
    int* cellRefs = new int[numCells] ();
    if(!MMG3D_Set_tetrahedra(mmgMesh, c, cellRefs))
        mexErrMsgTxt("MMG3D_Set_tetrahedra failed.");

    if(!MMG3D_Set_solSize(mmgMesh, mmgSol, MMG5_Vertex, numPoints, MMG5_Scalar))
        mexErrMsgTxt("MMG3D_Set_solSize failed.");
    
    if(!MMG3D_Set_scalarSols(mmgSol, s))
        mexErrMsgTxt("MMG3D_Set_scalarSols failed.");
    
    if(!MMG3D_Chk_meshData(mmgMesh, mmgSol))
        mexErrMsgTxt("mmgMesh and mmgSol are incompatible. Check if the number of point data array elements equals the number of points.");
    
    ///// Perform remeshing /////
    
    // ask for level set discretization
    if(!MMG3D_Set_iparameter(mmgMesh, mmgSol, MMG3D_IPARAM_iso, 1))
        mexErrMsgTxt("Setting MMG3D_IPARAM_iso failed.");

    // set isovalue
    if(!MMG3D_Set_dparameter(mmgMesh, mmgSol, MMG3D_DPARAM_ls, isoValue))
        mexErrMsgTxt("Setting MMG3D_DPARAM_ls failed.");
    
    // set hmin
    if(hmin > 0)
        if(!MMG3D_Set_dparameter(mmgMesh, mmgSol, MMG3D_DPARAM_hmin, hmin))
            mexErrMsgTxt("Setting MMG3D_DPARAM_hmin failed.");
    
    // set hmax
    if(hmax > 0)
        if(!MMG3D_Set_dparameter(mmgMesh, mmgSol, MMG3D_DPARAM_hmax, hmax))
            mexErrMsgTxt("Setting MMG3D_DPARAM_hmax failed.");
    
    // set hausd
    if(hausd > 0)
        if(!MMG3D_Set_dparameter(mmgMesh, mmgSol, MMG3D_DPARAM_hausd, hausd))
            mexErrMsgTxt("Setting MMG3D_DPARAM_hausd failed.");

    // perform actual remeshing
    int ier = MMG3D_mmg3dls(mmgMesh, mmgSol);
    switch(ier)
    {
        case MMG5_STRONGFAILURE:
            mexErrMsgTxt("mmg3dls failed with flag MMG5_STRONGFAILURE.");
        case MMG5_LOWFAILURE:
            mexWarnMsgTxt("mmg3dls failed with flag MMG5_LOWFAILURE.");
    }

    if(!MMG3D_saveMesh(mmgMesh, "/Users/steffen/Desktop/test.mesh"))
        mexErrMsgTxt("MMG3D_saveMesh failed.");

//TODO:
//    ///// Assemble output struct /////
//
//    int32_T numOutPoints, numOutCells;
//    if(!MMG3D_Get_meshSize(mmgMesh, &numOutPoints, &numOutCells, NULL, NULL, NULL, NULL))
//        mexErrMsgTxt("MMG3D_Get_meshSize failed.");
//
//    mexWarnMsgTxt(("numOutPoints = " + std::to_string(numOutPoints) + ", numOutCells = " + std::to_string(numOutCells)).c_str());
//
////    int  MMG3D_Get_vertices(mmgMesh, double* vertices, int* refs, int* areCorners, int* areRequired);
////
////    double* outPoints = new double[3*numPoints];
////
////
////    for(vtkIdType i = 0; i < numPoints; i++)
////        for(vtkIdType j = 0; j < 3; j++)
////            ((double*)mxGetData(points))[numPoints*j+i] = inPointSet->GetPoint(i)[j];
////
////    const char* fieldnames[6] = {"points", "cells", "cellTypes", "cellData"};
////    mxArray* outStruct = mxCreateStructMatrix(1, 1, 4, fieldnames);
////    mxSetField(outStruct, 0, "points", points);
////    mxSetField(outStruct, 0, "cells", cells);
////    mxSetField(outStruct, 0, "cellTypes", cellTypes);
////    mxSetField(outStruct, 0, "cellData", cellDataStruct);
////
////    plhs[0] = outStruct;

    ///// Free memory and close log file /////

    MMG3D_Free_all(MMG5_ARG_start, MMG5_ARG_ppMesh, &mmgMesh, MMG5_ARG_ppLs, &mmgSol, MMG5_ARG_end);
    
    delete[] p;
    delete[] c;
    delete[] s;
    delete[] pointRefs;
    delete[] cellRefs;

    if(logFile != NULL)
        fclose(stdout);
}
