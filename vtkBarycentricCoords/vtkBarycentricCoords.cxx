#include "structToVtk.h"
#include "vtkToStruct.h"

#include <vtkSmartPointer.h>
#include <vtkErrorCode.h>
#include <vtkPolyData.h>
#include <vtkPLYReader.h>
#include <vtkPLYWriter.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkPointLocator.h>
#include <vtkOBBTree.h>
#include <vtkIdList.h>
#include <vtkTriangle.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkPoints.h>

#include <string>
#include <sstream>
#include <map>

#include "Matrix3.h"
#include "Vector3.h"

using namespace math_pack;

void GetPointNormal(vtkPolyData* mesh, vtkIdType pointId, double normalLength, Vector3<double>& normal)
{
    vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();
    vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
    vtkSmartPointer<vtkIdList> neighborPointIds = vtkSmartPointer<vtkIdList>::New();
    mesh->GetPointCells(pointId, cellIds);
    
    normal = Vector3<double>(0, 0, 0);
    for(vtkIdType i = 0; i < cellIds->GetNumberOfIds(); i++)
    {
        vtkIdType cellId = cellIds->GetId(i);
        mesh->GetCellPoints(cellId, pointIds);
        if(pointIds->GetNumberOfIds() != 3)
            throw std::runtime_error("getPointNormal(): Cell " + std::to_string(cellId) + " is not a triangle.");
        double v[9];
        for(int i = 0; i < 3; i++)
        {
            vtkIdType neighborPointId = pointIds->GetId(i);
            double* vv = v+(3*i);
            mesh->GetPoint(neighborPointId, vv);
            if(neighborPointId != pointId)
                neighborPointIds->InsertUniqueId(neighborPointId);
        }
        Vector3<double> n;
        vtkTriangle::ComputeNormal(v, v+3, v+6, n.GetArray());
        normal += n;
    }
    normal.Normalize();
    normal *= normalLength;
}

void GetIntersection(vtkPolyData* mesh, vtkIdList* pointIds, Vector3<double> p, Vector3<double> n, Vector3<double>& x, double* barycentricCoords)
{
    Vector3<double> a, b, c, triangleNormal;
    mesh->GetPoint(pointIds->GetId(0), a.GetArray());
    mesh->GetPoint(pointIds->GetId(1), b.GetArray());
    mesh->GetPoint(pointIds->GetId(2), c.GetArray());
    vtkTriangle::ComputeNormal(a.GetArray(), b.GetArray(), c.GetArray(), triangleNormal.GetArray());
    
    double d = (a-p)*triangleNormal / (n*triangleNormal);
    x = p + d*n;
    
    Vector3<double> v0 = b-a, v1 = c-a, v2 = x-a;
    double d00 = v0*v0;
    double d01 = v0*v1;
    double d11 = v1*v1;
    double d20 = v2*v0;
    double d21 = v2*v1;
    double denom = d00*d11 - d01*d01;
    barycentricCoords[1] = (d11*d20 - d01*d21)/denom;
    barycentricCoords[2] = (d00*d21 - d01*d20)/denom;
    
    double tol = 1e-10;
    if(barycentricCoords[1] < -tol || barycentricCoords[1] > 1+tol || barycentricCoords[2] < -tol || barycentricCoords[2] > 1+tol)
        throw std::runtime_error("Barycentric coordinates out of range [0,1].");
    
    barycentricCoords[1] = (barycentricCoords[1] < 1) ? barycentricCoords[1] : 1;
    barycentricCoords[1] = (barycentricCoords[1] > 0) ? barycentricCoords[1] : 0;
    barycentricCoords[2] = (barycentricCoords[2] < 1) ? barycentricCoords[2] : 1;
    barycentricCoords[2] = (barycentricCoords[2] > 0) ? barycentricCoords[2] : 0;
    
    barycentricCoords[0] = 1.0 - barycentricCoords[1] - barycentricCoords[2];
}

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    double normalLength = 1.0;
    bool verbose = false;
    
    const std::string syntax = "[sourceCellIds, baryCoords, intersectionStruct, targetStruct] = vtkBarycentricCoords(sourceStruct, targetStruct, normalLength)";
    if(nrhs < 2)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 2)
        normalLength = mxGetScalar(prhs[2]);
    if(nrhs > 3)
        verbose = mxGetScalar(prhs[3]);
    if(nrhs > 4)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 4)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());

    ///// Convert MATLAB struct into vtkPointSet /////
    
    vtkSmartPointer<vtkPolyData> sourcePolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid1 = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet1 = structToVtk(prhs[0], sourcePolyData, unstructuredGrid1, false);
    if(sourcePolyData->GetDataObjectType() != VTK_POLY_DATA)
        mexErrMsgTxt("vtkBarycentricCoords requires poly data as first input. Incompatible cell(s) found.");
    vtkIdType numSourcePoints = sourcePolyData->GetNumberOfPoints();
    
    vtkSmartPointer<vtkPolyData> targetPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid2 = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet2 = structToVtk(prhs[1], targetPolyData, unstructuredGrid2, false);
    if(targetPolyData->GetDataObjectType() != VTK_POLY_DATA)
        mexErrMsgTxt("vtkBarycentricCoords requires poly data as second input. Incompatible cell(s) found.");
    vtkIdType numTargetPoints = targetPolyData->GetNumberOfPoints();
    
    ///// For each point in targetPolyData: Find triangle in sourcePolyData that is intersected by the line in normal direction passing through this point /////
    
    vtkSmartPointer<vtkDoubleArray> normalsArray = vtkSmartPointer<vtkDoubleArray>::New();
    normalsArray->SetNumberOfComponents(3);
    normalsArray->SetNumberOfTuples(targetPolyData->GetNumberOfPoints());
    normalsArray->SetName("normals");
    targetPolyData->GetPointData()->AddArray(normalsArray);
    
    vtkSmartPointer<vtkIntArray> notFoundArray = vtkSmartPointer<vtkIntArray>::New();
    notFoundArray->SetNumberOfComponents(1);
    notFoundArray->SetNumberOfTuples(targetPolyData->GetNumberOfPoints());
    notFoundArray->SetName("notFound");
    targetPolyData->GetPointData()->AddArray(notFoundArray);
    
    vtkSmartPointer<vtkIntArray> foundIdArray = vtkSmartPointer<vtkIntArray>::New();
    foundIdArray->SetNumberOfComponents(1);
    foundIdArray->SetNumberOfTuples(targetPolyData->GetNumberOfPoints());
    foundIdArray->SetName("foundId");
    targetPolyData->GetPointData()->AddArray(foundIdArray);
    
    vtkSmartPointer<vtkOBBTree> obbTree = vtkSmartPointer<vtkOBBTree>::New();
    obbTree->SetDataSet(sourcePolyData);
    obbTree->BuildLocator();
    //obbTree->SetTolerance(tolerance);
    
    vtkSmartPointer<vtkIdList> sourcePointIds = vtkSmartPointer<vtkIdList>::New();
    vtkIdType numNotFound = 0;
    
    vtkSmartPointer<vtkPolyData> intersectionMesh = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> intersectionPoints = vtkSmartPointer<vtkPoints>::New();
    intersectionMesh->SetPoints(intersectionPoints);
    
    mxArray* sourceCellIdsArray = mxCreateDoubleMatrix(numTargetPoints, 1, mxREAL);
    double* sourceCellIds = mxGetPr(sourceCellIdsArray);
    mxArray* baryCoordsArray = mxCreateDoubleMatrix(numTargetPoints, 3, mxREAL);
    double* baryCoords = mxGetPr(baryCoordsArray);
    
    for(vtkIdType id = 0; id < numTargetPoints; id++)
    {
        Vector3<double> n, p1, p2, p3;
        targetPolyData->GetPoint(id, p1.GetArray());
        GetPointNormal(targetPolyData, id, normalLength, n);
        normalsArray->SetTuple(id, n.GetArray());
        p2 = p1 + n;
        p3 = p1 - n;
        double t, x[3], pCoords[3];
        int subId;
        vtkIdType sourceCellId;
        int found = obbTree->IntersectWithLine(p1.GetArray(), p2.GetArray(), 0.0, t, x, pCoords, subId, sourceCellId);
        if(!found)
            found = obbTree->IntersectWithLine(p1.GetArray(), p3.GetArray(), 0.0, t, x, pCoords, subId, sourceCellId);
        if(!found)
        {
            notFoundArray->SetTuple1(id, 1);
            foundIdArray->SetTuple1(id, -1);
            if(verbose)
                mexWarnMsgTxt(("No source cell found for target point " + std::to_string(id+1)).c_str());
            
            sourceCellIds[id] = -1;
            baryCoords[id] = 0;
            baryCoords[numTargetPoints+id] = 0;
            baryCoords[2*numTargetPoints+id] = 0;
            
            numNotFound++;
        }
        else // found
        {
            notFoundArray->SetTuple1(id, 0);
            foundIdArray->SetTuple1(id, sourceCellId);
            
            sourcePolyData->GetCellPoints(sourceCellId, sourcePointIds);
            Vector3<double> x;
            double coords[3];
            GetIntersection(sourcePolyData, sourcePointIds, p1, n, x, coords);
            intersectionPoints->InsertNextPoint(x.GetArray());

            sourceCellIds[id] = sourceCellId+1;
            baryCoords[id] = coords[0];
            baryCoords[numTargetPoints+id] = coords[1];
            baryCoords[2*numTargetPoints+id] = coords[2];
        }
    }
    if(numNotFound > 0)
        mexWarnMsgTxt(("No source cell found for " + std::to_string(numNotFound) + " target points. Check field 'pointData.notFound' of the 4th output argument.").c_str());
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = sourceCellIdsArray;
    plhs[1] = baryCoordsArray;
    if(nlhs > 2)
        plhs[2] = vtkToStruct(intersectionMesh, false);
    if(nlhs > 3)
        plhs[3] = vtkToStruct(targetPolyData, false);
}
