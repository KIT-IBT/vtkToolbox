#include "structToVtk.h"
#include "vtkToStruct.h"

#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkKdTree.h>

// data array types
#include <vtkTypeInt8Array.h>
#include <vtkTypeUInt8Array.h>
#include <vtkTypeInt16Array.h>
#include <vtkTypeUInt16Array.h>
#include <vtkTypeInt32Array.h>
#include <vtkTypeUInt32Array.h>
#include <vtkTypeInt64Array.h>
#include <vtkTypeUInt64Array.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>

vtkDataArray* createDataArray(int dataType)
{
    switch(dataType)
    {
        case VTK_SIGNED_CHAR:
            return(vtkTypeInt8Array::New());
        case VTK_UNSIGNED_CHAR:
            return(vtkTypeUInt8Array::New());
        case VTK_SHORT:
            return(vtkTypeInt16Array::New());
        case VTK_UNSIGNED_SHORT:
            return(vtkTypeUInt16Array::New());
        case VTK_INT:
            return(vtkTypeInt32Array::New());
        case VTK_UNSIGNED_INT:
            return(vtkTypeUInt32Array::New());
        case VTK_LONG:
            return(vtkTypeInt64Array::New());
        case VTK_UNSIGNED_LONG:
            return(vtkTypeUInt64Array::New());
        case VTK_FLOAT:
            return(vtkFloatArray::New());
        case VTK_DOUBLE:
            return(vtkDoubleArray::New());
        default:
            mexWarnMsgTxt((std::string("Data array type ") + std::to_string(dataType) + " is not supported. Using double.").c_str());
            return(vtkDoubleArray::New());
    }
}

// Compute center points (centroids) of cells in a point set
void computeCenterPoints(vtkPointSet* pointSet, vtkPoints* centerPoints)
{
    centerPoints->SetNumberOfPoints(pointSet->GetNumberOfCells());
    for(vtkIdType i = 0; i < pointSet->GetNumberOfCells(); i++)
    {
        vtkCell* cell = pointSet->GetCell(i);
        double pcoords[3], centroid[3];
        double* weights = new double[cell->GetNumberOfPoints()];
        cell->GetParametricCenter(pcoords);
        int subId = 0;
        cell->EvaluateLocation(subId, pcoords, centroid, weights);
        delete[] weights;
        centerPoints->SetPoint(i, centroid);
    }
    centerPoints->Modified();
}

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    const std::string syntax = "outStruct = vtkArrayMapperNearestNeighbor(struct sourceStruct, struct targetStruct)";
    if(nrhs < 2)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 2)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());

    ///// Convert MATLAB struct into vtkPointSet /////
    
    vtkSmartPointer<vtkPolyData> sourcePolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> sourceUnstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* sourcePointSet = structToVtk(prhs[0], sourcePolyData, sourceUnstructuredGrid, false);
    
    vtkSmartPointer<vtkPolyData> targetPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> targetUnstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* targetPointSet = structToVtk(prhs[1], targetPolyData, targetUnstructuredGrid, false);
    
    ///// Map data arrays /////
    
    // prefix existing data in the target mesh with "old_"
    
    vtkCellData* targetCellData = targetPointSet->GetCellData();
    for(int i = 0; i < targetCellData->GetNumberOfArrays(); i++)
    {
        std::string newName = "old_" + std::string(targetCellData->GetArrayName(i));
        targetCellData->GetArray(i)->SetName(newName.c_str());
    }
    
    vtkPointData* targetPointData = targetPointSet->GetPointData();
    for(int i = 0; i < targetPointData->GetNumberOfArrays(); i++)
    {
        std::string newName = "old_" + std::string(targetPointData->GetArrayName(i));
        targetPointData->GetArray(i)->SetName(newName.c_str());
    }
    
    // mapping the data from cells and point by using the nearest neighbour
    
    vtkCellData* sourceCellData = sourcePointSet->GetCellData();
    for(int i = 0; i < sourceCellData->GetNumberOfArrays(); i++)
    {
        vtkDataArray* array = createDataArray(sourceCellData->GetArray(i)->GetDataType());
        array->SetName(sourceCellData->GetArray(i)->GetName());
        array->SetNumberOfComponents(sourceCellData->GetArray(i)->GetNumberOfComponents());
        array->SetNumberOfTuples(targetPointSet->GetNumberOfCells());
        targetCellData->AddArray(array);
    }
    
    vtkPointData* sourcePointData = sourcePointSet->GetPointData();
    for(int i = 0; i < sourcePointData->GetNumberOfArrays(); i++)
    {
        vtkDataArray* array = createDataArray(sourcePointData->GetArray(i)->GetDataType());
        array->SetName(sourcePointData->GetArray(i)->GetName());
        array->SetNumberOfComponents(sourcePointData->GetArray(i)->GetNumberOfComponents());
        array->SetNumberOfTuples(targetPointSet->GetNumberOfPoints());
        targetPointData->AddArray(array);
    }
    
    // map nearest neighbors arrays by name, useful if target mesh already has some arrays
    
    if(sourceCellData->GetNumberOfArrays() > 0)
    {
        vtkSmartPointer<vtkPoints> sourceCenterPoints = vtkSmartPointer<vtkPoints>::New();
        computeCenterPoints(sourcePointSet, sourceCenterPoints);
        
        vtkSmartPointer<vtkPoints> targetCenterPoints = vtkSmartPointer<vtkPoints>::New();
        computeCenterPoints(targetPointSet, targetCenterPoints);
        
        vtkSmartPointer<vtkKdTree> centerPointLocator = vtkSmartPointer<vtkKdTree>::New();
        centerPointLocator->BuildLocatorFromPoints(sourceCenterPoints);
        
        for(int j = 0; j < targetCellData->GetNumberOfArrays(); j++)
        {
            const char *name = targetCellData->GetArrayName(j);
            int k = -1;
            sourceCellData->GetArray(name, k);
            
            if(k != -1)
            {
                for(vtkIdType i = 0; i < targetPointSet->GetNumberOfCells(); i++)
                {
                    double *currentpoint = targetCenterPoints->GetPoint(i);
                    double dist;
                    vtkIdType sourceCellId = centerPointLocator->FindClosestPoint(currentpoint, dist);
                    
                    targetCellData->GetArray(name)->SetTuple(i, sourceCellData->GetArray(k)->GetTuple(sourceCellId));
                }
            }
        }
    }
    
    if(sourcePointData->GetNumberOfArrays() > 0)
    {
        vtkSmartPointer<vtkKdTree> cornerPointLocator = vtkSmartPointer<vtkKdTree>::New();
        cornerPointLocator->BuildLocatorFromPoints(sourcePointSet);
        
        for(int j = 0; j < targetPointData->GetNumberOfArrays(); j++)
        {
            const char *name = targetPointData->GetArrayName(j);
            int k = -1;
            sourcePointData->GetArray(name, k);
            
            if(k != -1)
            {
                for(vtkIdType i = 0; i < targetPointSet->GetNumberOfPoints(); i++)
                {
                    double *currentpoint  = targetPointSet->GetPoint(i);
                    double dist;
                    vtkIdType sourcePointId = cornerPointLocator->FindClosestPoint(currentpoint, dist);
                    
                    targetPointData->GetArray(name)->SetTuple(i, sourcePointData->GetArray(k)->GetTuple(sourcePointId));
                }
            }
        }
    }
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(targetPointSet, false);
}
