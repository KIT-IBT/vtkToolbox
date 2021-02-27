#include "structToVtk.h"

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

vtkDataArray* createVtkDataArray(mxClassID matlabDataType)
{
    switch(matlabDataType)
    {
        case mxLOGICAL_CLASS:
            return(vtkTypeUInt8Array::New());
        case mxCHAR_CLASS:
            return(vtkTypeUInt16Array::New());
        case mxINT8_CLASS:
            return(vtkTypeInt8Array::New());
        case mxUINT8_CLASS:
            return(vtkTypeUInt8Array::New());
        case mxINT16_CLASS:
            return(vtkTypeInt16Array::New());
        case mxUINT16_CLASS:
            return(vtkTypeUInt16Array::New());
        case mxINT32_CLASS:
            return(vtkTypeInt32Array::New());
        case mxUINT32_CLASS:
            return(vtkTypeUInt32Array::New());
        case mxINT64_CLASS:
            return(vtkTypeInt64Array::New());
        case mxUINT64_CLASS:
            return(vtkTypeUInt64Array::New());
        case mxSINGLE_CLASS:
            return(vtkFloatArray::New());
        case mxDOUBLE_CLASS:
            return(vtkDoubleArray::New());
        default:
            mexWarnMsgTxt((std::string("Data array type ") + std::to_string(matlabDataType) + " is not supported. Using double.").c_str());
            return(vtkDoubleArray::New());
    }
}

int getVtkDataType(mxClassID matlabDataType)
{
    switch(matlabDataType)
    {
        case mxLOGICAL_CLASS:
            return(VTK_UNSIGNED_CHAR);
        case mxCHAR_CLASS:
            return(VTK_UNSIGNED_SHORT);
        case mxINT8_CLASS:
            return(VTK_SIGNED_CHAR);
        case mxUINT8_CLASS:
            return(VTK_UNSIGNED_CHAR);
        case mxINT16_CLASS:
            return(VTK_SHORT);
        case mxUINT16_CLASS:
            return(VTK_UNSIGNED_SHORT);
        case mxINT32_CLASS:
            return(VTK_INT);
        case mxUINT32_CLASS:
            return(VTK_UNSIGNED_INT);
        case mxINT64_CLASS:
            return(VTK_LONG);
        case mxUINT64_CLASS:
            return(VTK_UNSIGNED_LONG);
        case mxSINGLE_CLASS:
            return(VTK_FLOAT);
        case mxDOUBLE_CLASS:
            return(VTK_DOUBLE);
        default:
            mexWarnMsgTxt((std::string("Data type ") + std::to_string(matlabDataType) + " is not supported. Using double.").c_str());
            return(VTK_DOUBLE);
    }
}

// This function is adapted from vtkMatlabMexAdapter
vtkDataArray* mxArrayToVtkDataArray(const mxArray* inArray)
{
    if(inArray == NULL)
    {
        mexErrMsgTxt("NULL input to mxArrayTovtkDataArray().");
        return(NULL);
    }
    if(mxGetNumberOfDimensions(inArray) > 2)
    {
        mexErrMsgTxt("Input to mxArrayTovtkDataArray() has more than two dimensions, cannot convert to vtkDataArray.");
        return(NULL);
    }
    if(mxIsCell(inArray))
    {
        mexErrMsgTxt("Input to mxArrayTovtkDataArray() is a Cell Array, cannot convert to vtkDataArray.");
        return(NULL);
    }
    if(mxIsSparse(inArray))
    {
        mexErrMsgTxt("Input to mxArrayTovtkDataArray() is a Sparse Array, cannot convert to vtkDataArray.");
        return(NULL);
    }
    
    size_t numRows = mxGetM(inArray);
    size_t numColumns = mxGetN(inArray);
    
    vtkDataArray* outArray = createVtkDataArray(mxGetClassID(inArray));
    
    size_t numBytes = mxGetElementSize(inArray);
    if(numBytes != outArray->GetElementComponentSize())
    {
        outArray->Delete();
        mexErrMsgTxt("Data size mismatch between Matlab and VTK.");
        return(NULL);
    }
    
    outArray->SetNumberOfTuples(numRows);
    outArray->SetNumberOfComponents((int)numColumns);
    
    double* tuple = (double*) mxMalloc(sizeof(double)*numColumns);
    unsigned char* source = (unsigned char*)mxGetData(inArray);
    unsigned char* dest;
    
    for(size_t i = 0; i < numRows; i++)
    {
        outArray->InsertTuple(i,tuple);
        for(size_t j = 0; j < numColumns; j++)
        {
            dest = (unsigned char*) outArray->GetVoidPointer(i*numColumns + j);
            for(size_t k = 0; k < numBytes; k++)
                dest[k] = source[j*(numRows*numBytes) + i*numBytes + k];
        }
    }
    mxFree(tuple);
    return(outArray);
}

int isPolyDataCell(uint8_t cellType)
{
    switch(cellType)
    {
        case VTK_VERTEX:
            return(true);
        case VTK_POLY_VERTEX:
            return(true);
        case VTK_LINE:
            return(true);
        case VTK_POLY_LINE:
            return(true);
        case VTK_TRIANGLE:
            return(true);
        case VTK_QUAD:
            return(true);
        case VTK_POLYGON:
            return(true);
        case VTK_TRIANGLE_STRIP:
            return(true);
        default:
            return(false);
    }
}

vtkPointSet* structToVtk(const mxArray* inStruct, vtkPolyData* polyData, vtkUnstructuredGrid* unstructuredGrid, bool verbose)
{
    ///// Read input struct /////
    
    printVerbose("Reading input struct...\n", verbose);
    
    mxArray* points = mxGetField(inStruct, 0, "points");
    if(points == NULL)
        mexErrMsgTxt("Field 'points' of input struct could not be read.");
    int vtkDataTypeOfPoints = getVtkDataType(mxGetClassID(points));

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
    mxArray* cellDataStruct = mxGetField(inStruct, 0, "cellData");
    mxArray* fieldDataStruct = mxGetField(inStruct, 0, "fieldData");
    
    ///// Check file format compatibility /////
    
    // Check, whether there are only cells compatible with vtkPolyData
    bool isPolyData = true;
    for(size_t i = 0; i < mxGetNumberOfElements(cellTypes); i++)
    {
        uint8_t cellType = ((uint8_t*)mxGetData(cellTypes))[i];
        if(!isPolyDataCell(cellType))
        {
            isPolyData = false;
            break;
        }
    }
    
    ///// Process cells /////
    
    printVerbose("Processing cells...\n", verbose);
    
    mwSize numCells = mxGetM(cells);
    if(mxGetM(cellTypes)*mxGetN(cellTypes) != numCells)
        mexErrMsgTxt("The number of elements in 'cellTypes' must be equal to the number of rows in 'cells'.");
        
    mwSize maxNumCellPoints = mxGetN(cells);
    vtkIdType* pointIds = new vtkIdType[maxNumCellPoints];
    
    vtkPointSet* outPointSet;
    
    if(isPolyData)
    {
        polyData->Allocate(numCells);
        for(vtkIdType i = 0; i < numCells; i++)
        {
            uint8_t cellType = ((uint8_t*)mxGetData(cellTypes))[i];
            int numCellPoints = getNumberOfCellPoints(cellType);
            for(vtkIdType j = 0; j < numCellPoints; j++)
            {
                pointIds[j] = ((int32_t*)mxGetData(cells))[numCells*j+i]-1;
            }
            polyData->InsertNextCell(cellType, numCellPoints, pointIds);
        }
        outPointSet = polyData;
    }
    else
    {
        unstructuredGrid->Allocate(numCells);
        for(vtkIdType i = 0; i < numCells; i++)
        {
            uint8_t cellType = ((uint8_t*)mxGetData(cellTypes))[i];
            int numCellPoints = getNumberOfCellPoints(cellType);
            for(vtkIdType j = 0; j < numCellPoints; j++)
            {
                pointIds[j] = ((int32_t*)mxGetData(cells))[numCells*j+i]-1;
            }
            unstructuredGrid->InsertNextCell(cellType, numCellPoints, pointIds);
        }
        outPointSet = unstructuredGrid;
    }
    
    delete[] pointIds;
    
    ///// Process points /////
    
    printVerbose("Processing points...\n", verbose);
    
    vtkSmartPointer<vtkPoints> vtkpoints = vtkSmartPointer<vtkPoints>::New();
    vtkpoints->SetDataType(vtkDataTypeOfPoints);
    vtkIdType numPoints = mxGetM(points);
    vtkpoints->SetNumberOfPoints(numPoints);
    
    switch(vtkDataTypeOfPoints)
    {
        case VTK_SIGNED_CHAR:
            for(vtkIdType i = 0; i < numPoints; i++) { double pointCoords[3]; for(int j = 0; j < 3; j++) pointCoords[j] = ((int8_T*)mxGetData(points))[numPoints*j+i]; vtkpoints->SetPoint(i, pointCoords); }
            break;
        case VTK_UNSIGNED_CHAR:
            for(vtkIdType i = 0; i < numPoints; i++) { double pointCoords[3]; for(int j = 0; j < 3; j++) pointCoords[j] = ((uint8_T*)mxGetData(points))[numPoints*j+i]; vtkpoints->SetPoint(i, pointCoords); }
            break;
        case VTK_SHORT:
            for(vtkIdType i = 0; i < numPoints; i++) { double pointCoords[3]; for(int j = 0; j < 3; j++) pointCoords[j] = ((int16_T*)mxGetData(points))[numPoints*j+i]; vtkpoints->SetPoint(i, pointCoords); }
            break;
        case VTK_UNSIGNED_SHORT:
            for(vtkIdType i = 0; i < numPoints; i++) { double pointCoords[3]; for(int j = 0; j < 3; j++) pointCoords[j] = ((uint16_T*)mxGetData(points))[numPoints*j+i]; vtkpoints->SetPoint(i, pointCoords); }
            break;
        case VTK_INT:
            for(vtkIdType i = 0; i < numPoints; i++) { double pointCoords[3]; for(int j = 0; j < 3; j++) pointCoords[j] = ((int32_T*)mxGetData(points))[numPoints*j+i]; vtkpoints->SetPoint(i, pointCoords); }
            break;
        case VTK_UNSIGNED_INT:
            for(vtkIdType i = 0; i < numPoints; i++) { double pointCoords[3]; for(int j = 0; j < 3; j++) pointCoords[j] = ((uint32_T*)mxGetData(points))[numPoints*j+i]; vtkpoints->SetPoint(i, pointCoords); }
            break;
        case VTK_LONG:
            for(vtkIdType i = 0; i < numPoints; i++) { double pointCoords[3]; for(int j = 0; j < 3; j++) pointCoords[j] = ((int64_T*)mxGetData(points))[numPoints*j+i]; vtkpoints->SetPoint(i, pointCoords); }
            break;
        case VTK_UNSIGNED_LONG:
            for(vtkIdType i = 0; i < numPoints; i++) { double pointCoords[3]; for(int j = 0; j < 3; j++) pointCoords[j] = ((uint64_T*)mxGetData(points))[numPoints*j+i]; vtkpoints->SetPoint(i, pointCoords); }
            break;
        case VTK_FLOAT:
            for(vtkIdType i = 0; i < numPoints; i++) { double pointCoords[3]; for(int j = 0; j < 3; j++) pointCoords[j] = ((float*)mxGetData(points))[numPoints*j+i]; vtkpoints->SetPoint(i, pointCoords); }
            break;
        case VTK_DOUBLE:
            for(vtkIdType i = 0; i < numPoints; i++) { double pointCoords[3]; for(int j = 0; j < 3; j++) pointCoords[j] = ((double*)mxGetData(points))[numPoints*j+i]; vtkpoints->SetPoint(i, pointCoords); }
            break;
        default:
            mexErrMsgTxt("Unknown VTK data type of points.");
    }
    
    vtkpoints->Modified();
    outPointSet->SetPoints(vtkpoints);
    
    ///// Process point data /////
    
    if(pointDataStruct != NULL)
    {
        printVerbose("Processing point data...\n", verbose);
        
        int numPointDataArrays = mxGetNumberOfFields(pointDataStruct);
        for(int i = 0; i < numPointDataArrays; i++)
        {
            const char* arrayName = mxGetFieldNameByNumber(pointDataStruct, i);
            mxArray* array = mxGetFieldByNumber(pointDataStruct, 0, i);
            size_t numRows = mxGetM(array);
            if(numRows != numPoints)
                mexErrMsgTxt((std::string("Wrong number of rows in point data array '") + std::string(arrayName) + "'.").c_str());
            vtkDataArray* pointDataArray = mxArrayToVtkDataArray(array);
            pointDataArray->SetName(arrayName);
            outPointSet->GetPointData()->AddArray(pointDataArray);
        }
        
        if(outPointSet->GetDataObjectType() == VTK_POLY_DATA && outPointSet->GetPointData()->HasArray("Normals"))
            outPointSet->GetPointData()->SetNormals(outPointSet->GetPointData()->GetArray("Normals"));
    }
    
    ///// Process cell data /////
    
    if(cellDataStruct != NULL)
    {
        printVerbose("Processing cell data...\n", verbose);
        
        int numCellDataArrays = mxGetNumberOfFields(cellDataStruct);
        for(int i = 0; i < numCellDataArrays; i++)
        {
            const char* arrayName = mxGetFieldNameByNumber(cellDataStruct, i);
            mxArray* array = mxGetFieldByNumber(cellDataStruct, 0, i);
            size_t numRows = mxGetM(array);
            if(numRows != numCells)
                mexErrMsgTxt((std::string("Wrong number of rows in cell data array '") + std::string(arrayName) + "'.").c_str());
            
            vtkDataArray* cellDataArray = mxArrayToVtkDataArray(array);
            cellDataArray->SetName(arrayName);
            outPointSet->GetCellData()->AddArray(cellDataArray);
        }
        
        if(outPointSet->GetDataObjectType() == VTK_POLY_DATA && outPointSet->GetCellData()->HasArray("Normals"))
            outPointSet->GetCellData()->SetNormals(outPointSet->GetCellData()->GetArray("Normals"));
    }
    
    ///// Process field data /////
    
    if(fieldDataStruct != NULL)
    {
        printVerbose("Processing field data...\n", verbose);
        
        int numFieldDataArrays = mxGetNumberOfFields(fieldDataStruct);
        for(int i = 0; i < numFieldDataArrays; i++)
        {
            const char* arrayName = mxGetFieldNameByNumber(fieldDataStruct, i);
            mxArray* array = mxGetFieldByNumber(fieldDataStruct, 0, i);
            vtkDataArray* fieldDataArray = mxArrayToVtkDataArray(array);
            fieldDataArray->SetName(arrayName);
            outPointSet->GetFieldData()->AddArray(fieldDataArray);
        }
    }
    
    /////
    
    return outPointSet;
}
