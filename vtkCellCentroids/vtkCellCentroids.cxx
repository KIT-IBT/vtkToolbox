#include "structToVtk.h"
#include "vtkToStruct.h"

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    const std::string syntax = "outStruct = vtkCellCentroids(inStruct)";
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
    
    ///// Compute cell centroids /////
    
    vtkSmartPointer<vtkPoints> centroidPoints = vtkSmartPointer<vtkPoints>::New();
    vtkIdType numCells = pointSet->GetNumberOfCells();
    centroidPoints->SetNumberOfPoints(numCells);
    
    for(vtkIdType i = 0; i < numCells; i++)
    {
        vtkCell* cell = pointSet->GetCell(i);
        double pcoords[3], centroid[3];
        double* weights = new double[cell->GetNumberOfPoints()];
        cell->GetParametricCenter(pcoords);
        int subId = 0;
        cell->EvaluateLocation(subId, pcoords, centroid, weights);
        delete[] weights;
        centroidPoints->SetPoint(i, centroid);
    }
    centroidPoints->Modified();
    
    vtkSmartPointer<vtkPolyData> cellCentroids = vtkSmartPointer<vtkPolyData>::New();
    cellCentroids->SetPoints(centroidPoints);
    vtkCellData* cellData = pointSet->GetCellData();
    for(int i = 0; i < cellData->GetNumberOfArrays(); i++)
        cellCentroids->GetPointData()->AddArray(cellData->GetArray(i));
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(cellCentroids, false);
}
