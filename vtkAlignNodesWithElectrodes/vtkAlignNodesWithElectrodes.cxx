#include "structToVtk.h"
#include "vtkToStruct.h"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPolyDataNormals.h>
#include <vtkPointLocator.h>
#include <vtkOBBTree.h>
#include <vtkTriangle.h>
#include <vtkIdList.h>

#include "Matrix3.h"
#include "Vector3.h"

using namespace math_pack;

void GetIntersection(vtkPolyData* mesh, vtkIdList* pointIds, Vector3<double> p, Vector3<double> n, Vector3<double>& x)
{
    Vector3<double> a, b, c, triangleNormal;
    mesh->GetPoint(pointIds->GetId(0), a.GetArray());
    mesh->GetPoint(pointIds->GetId(1), b.GetArray());
    mesh->GetPoint(pointIds->GetId(2), c.GetArray());
    vtkTriangle::ComputeNormal(a.GetArray(), b.GetArray(), c.GetArray(), triangleNormal.GetArray());
    
    double d = (a-p)*triangleNormal / (n*triangleNormal);
    x = p + d*n;
}

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    const std::string syntax = "outStruct = vtkAlignNodesWithElectrodes(bodyStruct, electrodesStruct)";
    if(nrhs < 2)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 2)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());

    ///// Convert MATLAB struct into vtkPointSet /////
    
    vtkSmartPointer<vtkPolyData> polyData1 = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid1 = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet1 = structToVtk(prhs[0], polyData1, unstructuredGrid1, false);
    if(polyData1->GetDataObjectType() != VTK_POLY_DATA)
        mexErrMsgTxt("vtkAlignNodesWithElectrodes requires poly data as first input. Incompatible cell(s) found.");
    
    vtkSmartPointer<vtkPolyData> polyData2 = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid2 = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet2 = structToVtk(prhs[1], polyData2, unstructuredGrid2, false);
    if(polyData1->GetDataObjectType() != VTK_POLY_DATA)
        mexErrMsgTxt("vtkAlignNodesWithElectrodes requires poly data as second input. Incompatible cell(s) found.");
    
    //////////
    
    vtkSmartPointer<vtkPolyDataNormals> polyDataNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
    polyDataNormals->SetInputData(polyData1);
    polyDataNormals->ComputePointNormalsOn();
    polyDataNormals->Update();
    vtkDataArray* normalsArray = polyDataNormals->GetOutput()->GetPointData()->GetArray("Normals");
    
    vtkSmartPointer<vtkPointLocator> pointLocator = vtkSmartPointer<vtkPointLocator>::New();
    pointLocator->SetDataSet(polyData1);
    pointLocator->BuildLocator();
    
    vtkSmartPointer<vtkOBBTree> obbTree = vtkSmartPointer<vtkOBBTree>::New();
    obbTree->SetDataSet(polyData1);
    obbTree->BuildLocator();
    
    for(vtkIdType i = 0; i < polyData2->GetNumberOfPoints(); i++)
    {
        Vector3<double> electrodeCoords;
        polyData2->GetPoint(i, electrodeCoords.GetArray());
        vtkIdType closestPointId = pointLocator->FindClosestPoint(electrodeCoords.GetArray());
        
        Vector3<double> n, p1, p2;
        normalsArray->GetTuple(closestPointId, n.GetArray());
        n.Normalize();
        p1 = electrodeCoords - 20.0*n;
        p2 = electrodeCoords + 20.0*n;
        
        double t, x[3], pCoords[3];
        int subId;
        vtkIdType torsoCellId;
        int found = obbTree->IntersectWithLine(p1.GetArray(), p2.GetArray(), 0.0, t, x, pCoords, subId, torsoCellId);
        if(!found)
            throw std::runtime_error("No triangle found during line intersection.");
        
        vtkSmartPointer<vtkIdList> torsoPointIds = vtkSmartPointer<vtkIdList>::New();
        polyData1->GetCellPoints(torsoCellId, torsoPointIds);
        Vector3<double> intersectionCoords;
        GetIntersection(polyData1, torsoPointIds, p1, n, intersectionCoords);
        
        polyData1->GetPoints()->SetPoint(closestPointId, intersectionCoords.GetArray());
    }
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(polyData1, false);
}
