#include "structToVtk.h"
#include "vtkToStruct.h"

#include <vtkGradientFilter.h>
#include <vtkStreamTracer.h>
#include <vtkTriangleFilter.h>

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    ///// Parse inputs /////
    
    const std::string syntax = "outStruct = vtkStreamTracer(inStruct1, cellsOrPoints, arrayName, inStruct2, direction, stepLength, endPointPrecision, maxLength)";
    if(nrhs > 8)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nrhs < 6)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    std::string directionString = std::string(mxArrayToString(prhs[4]));
    int direction;
    if(directionString == "forward")
        direction = vtkStreamTracer::FORWARD;
    else if(directionString == "backward")
        direction = vtkStreamTracer::BACKWARD;
    else if(directionString == "both")
        direction = vtkStreamTracer::BOTH;
    else
        mexErrMsgTxt(("Unknown direction \"" + directionString + "\".").c_str());
    double stepLength = mxGetScalar(prhs[5]);
    double endPointPrecision = -1;
    double maxLength = 1e3*stepLength;
    if(nrhs > 7)
        maxLength = mxGetScalar(prhs[7]);
    if(nrhs > 6)
        endPointPrecision = mxGetScalar(prhs[6]);
    if(nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());

    ///// Convert MATLAB struct into vtkPointSet /////
    
    vtkSmartPointer<vtkPolyData> polyData1 = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid1 = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet1 = structToVtk(prhs[0], polyData1, unstructuredGrid1, false);
    
    vtkSmartPointer<vtkPolyData> polyData2 = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid2 = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet2 = structToVtk(prhs[3], polyData2, unstructuredGrid2, false);
    
    ///// Apply vtkGradientFilter and vtkStreamTracer /////
    
    if(pointSet1->GetDataObjectType() == VTK_POLY_DATA)
        mexErrMsgTxt("vtkStreamTracer requires an unstructured grid as first input.");
    if(pointSet2->GetDataObjectType() != VTK_POLY_DATA)
        mexErrMsgTxt("vtkStreamTracer requires poly data as second input. Incompatible cell(s) found.");
    
    vtkSmartPointer<vtkGradientFilter> gradientFilter = vtkSmartPointer<vtkGradientFilter>::New();
    vtkSmartPointer<vtkStreamTracer> streamTracer = vtkSmartPointer<vtkStreamTracer>::New();
    
    char* cellsOrPoints = mxArrayToString(prhs[1]);
    char* arrayName = mxArrayToString(prhs[2]);
    vtkDataArray* dataArray = getDataArray(pointSet1, cellsOrPoints, arrayName);
    if(!dataArray)
        mexErrMsgTxt(("Data array \"" + std::string(arrayName) + "\" could not be found. Also check the argument cellsOrPoints.").c_str());
    
    if(strcmp(cellsOrPoints, "cells") == 0)
    {
        if(dataArray->GetNumberOfComponents() == 1)
        {
            // scalar field provided: first compute its gradient and then apply the stream tracer
            gradientFilter->SetInputData(pointSet1);
            gradientFilter->SetInputScalars(vtkDataObject::FIELD_ASSOCIATION_CELLS, arrayName);
            std::string gradientArrayName = std::string(arrayName) + "_gradient";
            gradientFilter->SetResultArrayName(gradientArrayName.c_str());
            gradientFilter->Update();
            streamTracer->SetInputData(gradientFilter->GetOutput());
            streamTracer->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, gradientArrayName.c_str());
        }
        else
        {
            // vector field provided: directly apply the stream tracer
            streamTracer->SetInputData(pointSet1);
            streamTracer->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, arrayName);
        }
    }
    else if(strcmp(cellsOrPoints, "points") == 0)
    {
        if(dataArray->GetNumberOfComponents() == 1)
        {
            // scalar field provided: first compute its gradient and then apply the stream tracer
            gradientFilter->SetInputData(pointSet1);
            gradientFilter->SetInputScalars(vtkDataObject::FIELD_ASSOCIATION_POINTS, arrayName);
            std::string gradientArrayName = std::string(arrayName) + "_gradient";
            gradientFilter->SetResultArrayName(gradientArrayName.c_str());
            gradientFilter->Update();
            streamTracer->SetInputData(gradientFilter->GetOutput());
            streamTracer->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, gradientArrayName.c_str());
        }
        else
        {
            // vector field provided: directly apply the stream tracer
            streamTracer->SetInputData(pointSet1);
            streamTracer->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, arrayName);
        }
    }
    else
        mexErrMsgTxt("cellsOrPoints must be \"cells\" or \"points\".");
    
    streamTracer->SetSourceData(pointSet2);
    streamTracer->SurfaceStreamlinesOff();
    streamTracer->SetIntegratorTypeToRungeKutta4();
    streamTracer->SetIntegrationDirection(direction);
    streamTracer->SetIntegrationStepUnit(vtkStreamTracer::LENGTH_UNIT);
    streamTracer->SetInitialIntegrationStep(stepLength);
    streamTracer->SetMaximumPropagation(maxLength);
    streamTracer->SetComputeVorticity(false);
    streamTracer->Update();
    
    vtkSmartPointer<vtkPolyData> sl1 = vtkSmartPointer<vtkPolyData>::New();
    
    // Restart stream tracer using a smaller step length (= endPointPrecision) at the end points of
    // the first run and replace these points by the end points of the second run.
    // This can be used to reduce the gap between the end of the streamline and the domain boundary.
    if(endPointPrecision > 0 && endPointPrecision < stepLength)
    {
        sl1->DeepCopy(streamTracer->GetOutput());
        
        vtkSmartPointer<vtkPoints> secondarySeedPoints = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkPolyData> secondarySeeds = vtkSmartPointer<vtkPolyData>::New();
        secondarySeeds->SetPoints(secondarySeedPoints);
        secondarySeedPoints->SetNumberOfPoints(sl1->GetNumberOfCells());
        
        vtkDataArray* reasonForTermination = sl1->GetCellData()->GetArray("ReasonForTermination");
        for(vtkIdType i = 0; i < sl1->GetNumberOfCells(); i++)
        {
            if(reasonForTermination->GetTuple1(i) != vtkStreamTracer::OUT_OF_DOMAIN)
                continue;
            vtkSmartPointer<vtkIdList> ptIds = vtkSmartPointer<vtkIdList>::New();
            sl1->GetCellPoints(i, ptIds);
            vtkIdType endPtId = ptIds->GetId(ptIds->GetNumberOfIds()-1);
            secondarySeedPoints->SetPoint(i, sl1->GetPoint(endPtId));
        }
        
        streamTracer->SetSourceData(secondarySeeds);
        streamTracer->SetInitialIntegrationStep(endPointPrecision);
        streamTracer->Update();
        vtkPolyData* sl2  = streamTracer->GetOutput();
        
        vtkPointData* ptDataSl1 = sl1->GetPointData();
        vtkPointData* ptDataSl2 = sl2->GetPointData();
        int numPointDataArrays = ptDataSl1->GetNumberOfArrays();
        
        vtkDataArray* seedIds = sl2->GetCellData()->GetArray("SeedIds");
        for(vtkIdType i = 0; i < sl2->GetNumberOfCells(); i++)
        {
            vtkSmartPointer<vtkIdList> ptIds = vtkSmartPointer<vtkIdList>::New();
            sl1->GetCellPoints(seedIds->GetTuple1(i), ptIds);
            vtkIdType endPtIdSl1 = ptIds->GetId(ptIds->GetNumberOfIds()-1);
            sl2->GetCellPoints(i, ptIds);
            vtkIdType endPtIdSl2 = ptIds->GetId(ptIds->GetNumberOfIds()-1);
            sl1->GetPoints()->SetPoint(endPtIdSl1, sl2->GetPoint(endPtIdSl2));
            
            for(int j = 0; j < numPointDataArrays; j++)
            {
                if(strcmp(ptDataSl1->GetArrayName(j), "IntegrationTime") == 0)
                {
                    double it1 = ptDataSl2->GetArray(j)->GetTuple1(endPtIdSl2);
                    double it2 = ptDataSl1->GetArray(j)->GetTuple1(endPtIdSl1);
                    ptDataSl1->GetArray(j)->SetTuple1(endPtIdSl1, it1+it2);
                }
                else
                {
                    double* tuple = ptDataSl2->GetArray(j)->GetTuple(endPtIdSl2);
                    ptDataSl1->GetArray(j)->SetTuple(endPtIdSl1, tuple);
                }
            }
        }
    }
    else
        sl1->ShallowCopy(streamTracer->GetOutput());
    
    vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
    triangleFilter->SetInputData(sl1);
    triangleFilter->PassLinesOn();
    triangleFilter->Update();
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(triangleFilter->GetOutput(), false);
}
