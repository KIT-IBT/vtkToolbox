#include <vtkSmartPointer.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLPolyDataReader.h>

#include <string>
#include <cmath>
#include <vector>
#include <algorithm>

//temporal:
#include <vtkXMLPolyDataReader.h>
#include <vtkPLYReader.h>
//new:
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkPointSource.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkIdList.h>
#include <vtkKdTreePointLocator.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>


#include "vtkToStruct.h"
#include "structToVtk.h"
mxArray* vtkDataArrayToMxArray(vtkDataArray* inArray);
vtkDataArray* mxArrayToVtkDataArray(const mxArray* inArray);

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    
    const std::string syntax = "outStruct = vtkTEMPLATE(inStruct)";
    ///// Parse inputs /////
    if (nrhs < 3)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if (nrhs > 3)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if (nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());

    ///// Convert MATLAB struct into vtkPointSet /////
    
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet = structToVtk(prhs[0], polyData, unstructuredGrid, false);
    vtkDataArray* seedPoints = mxArrayToVtkDataArray(prhs[1]);
    double radius = mxGetScalar(prhs[2]);
    
    ///// Building KDPointTree /////
    
    vtkSmartPointer<vtkKdTreePointLocator> pointTreeOriginal = vtkSmartPointer<vtkKdTreePointLocator>::New();
    pointTreeOriginal->SetDataSet(pointSet);
    pointTreeOriginal->BuildLocator();
    
    ///// Get points within radius /////
    
    vtkSmartPointer<vtkIdList> result_total = vtkSmartPointer<vtkIdList>::New();
    for (vtkIdType i=0;i<seedPoints->GetNumberOfTuples();i++)
    {
        double* p = seedPoints->GetTuple3(i);
        vtkSmartPointer<vtkIdList> res_tmp = vtkSmartPointer<vtkIdList>::New();
        pointTreeOriginal->FindPointsWithinRadius(radius, p, res_tmp);
        
        ///// Remove duplicates /////
        
        for(vtkIdType j = 0; j < res_tmp->GetNumberOfIds(); j++)
        {
            vtkIdType tmp_id = res_tmp->GetId(j);
            double p[3];
            pointSet->GetPoint(tmp_id, p);
            if (result_total->IsId(tmp_id) == -1)
            {
                result_total->InsertNextId(tmp_id);
            }
        }
    }
    
    ///// Convert vtkIdList to vtkDataArray for MATLAB /////
    vtkDataArray* ids_out = vtkDoubleArray::New();
    for(vtkIdType i = 0; i < result_total->GetNumberOfIds(); i++)
    {
        vtkIdType point_ind = result_total->GetId(i);
        double p[3];
        pointSet->GetPoint(point_ind,p);
        ids_out->InsertTuple1(i,double(point_ind+1));
    }
    plhs[0] = vtkDataArrayToMxArray(ids_out);
}
