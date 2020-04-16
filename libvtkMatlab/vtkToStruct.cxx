#include "vtkToStruct.h"

mxClassID getMatlabDataType(int vtkDataType)
{
    switch(vtkDataType)
    {
        case VTK_BIT:
            return(mxUINT8_CLASS);
        case VTK_CHAR:
            #ifdef VTK_TYPE_CHAR_IS_SIGNED
                return(mxINT8_CLASS);
            #else
                return(mxUINT8_CLASS);
            #endif
        case VTK_SIGNED_CHAR:
            return(mxINT8_CLASS);
        case VTK_UNSIGNED_CHAR:
            return(mxUINT8_CLASS);
        case VTK_SHORT:
            return(mxINT16_CLASS);
        case VTK_UNSIGNED_SHORT:
            return(mxUINT16_CLASS);
        case VTK_INT:
            return(mxINT32_CLASS);
        case VTK_UNSIGNED_INT:
            return(mxUINT32_CLASS);
        case VTK_LONG:
            return(mxINT64_CLASS);
        case VTK_UNSIGNED_LONG:
            return(mxUINT64_CLASS);
        case VTK_LONG_LONG:
            return(mxINT64_CLASS);
        case VTK_UNSIGNED_LONG_LONG:
            return(mxUINT64_CLASS);
        case VTK_ID_TYPE:
            #ifdef VTK_USE_64BIT_IDS
                return(mxINT64_CLASS);
            #else
                return(mxINT32_CLASS);
            #endif
        case VTK_FLOAT:
            return(mxSINGLE_CLASS);
        case VTK_DOUBLE:
            return(mxDOUBLE_CLASS);
        default:
            mexWarnMsgTxt((std::string("Data type ") + std::to_string(vtkDataType) + " is not supported. Using double.").c_str());
            return(mxDOUBLE_CLASS);
    }
}

// This function is adapted from vtkMatlabMexAdapter
mxArray* vtkDataArrayToMxArray(vtkDataArray* inArray)
{
    mxClassID matlabDataType = getMatlabDataType(inArray->GetDataType());

    mwSize numTuples = inArray->GetNumberOfTuples();
    mwSize numComponents = inArray->GetNumberOfComponents();
    
    mxArray* outArray = mxCreateNumericMatrix(numTuples, numComponents, matlabDataType, mxREAL);
    mwSize numBytes = mxGetElementSize(outArray);
    
    if(numBytes != inArray->GetElementComponentSize())
        mexErrMsgTxt("Data size mismatch between Matlab and VTK.");
    
    unsigned char* source;
    unsigned char* dest = (unsigned char*)mxGetData(outArray);
    for(int i = 0; i < numTuples; i++)
    {
        for(int j = 0; j < numComponents; j++)
        {
            source = (unsigned char*)inArray->GetVoidPointer(i*numComponents + j);
            for(int k = 0; k < numBytes; k++)
                dest[(numTuples*numBytes)*j + numBytes*i + k] = source[k];
        }
    }
    return(outArray);
}

mxArray* vtkToStruct(vtkPointSet* inPointSet, bool verbose)
{
    ///// Process points and cells /////
    
    vtkSmartPointer<vtkCellTypes> typesOfCells = vtkSmartPointer<vtkCellTypes>::New();
    inPointSet->GetCellTypes(typesOfCells);
    
    if(typesOfCells->IsType(VTK_POLY_VERTEX) || typesOfCells->IsType(VTK_POLY_LINE) || typesOfCells->IsType(VTK_TRIANGLE_STRIP) || typesOfCells->IsType(VTK_POLYGON))
    {
        mexWarnMsgTxt("Found one of the following: vtkPolyVertex, vtkPolyLine, vtkTriangleStrip, vtkPolygon. Applying vtkTriangleFilter to convert these cells - this modifies the dataset!");
        vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
        triangleFilter->SetInputData(inPointSet);
        triangleFilter->Update();
        inPointSet = triangleFilter->GetOutput();
        inPointSet->GetCellTypes(typesOfCells);
    }
    
    mwSize numPoints = inPointSet->GetNumberOfPoints();
    mwSize numCells = inPointSet->GetNumberOfCells();
    
    mwSize maxPointsPerCell = 0;
    for(int i = 0; i < typesOfCells->GetNumberOfTypes(); i++)
    {
        unsigned char cellType = typesOfCells->GetCellType(i);
        mwSize n = getNumberOfCellPoints(cellType);
        maxPointsPerCell = std::max(n, maxPointsPerCell);
    }
    
    mxClassID matlabDataTypeOfPoints = mxDOUBLE_CLASS;
    if(numPoints)
        matlabDataTypeOfPoints = getMatlabDataType(inPointSet->GetPoints()->GetDataType());
    mxArray* points    = mxCreateNumericMatrix(numPoints, 3, matlabDataTypeOfPoints, mxREAL);
    mxArray* cells     = mxCreateNumericMatrix(numCells, maxPointsPerCell, mxINT32_CLASS, mxREAL);
    mxArray* cellTypes = mxCreateNumericMatrix(numCells, 1, mxUINT8_CLASS, mxREAL);
    
    printVerbose("Processing points...\n", verbose);
    
    switch(matlabDataTypeOfPoints)
    {
        case mxINT8_CLASS:
            for(vtkIdType i = 0; i < numPoints; i++) for(vtkIdType j = 0; j < 3; j++) ((int8_T*)mxGetData(points))[numPoints*j+i] = inPointSet->GetPoint(i)[j];
            break;
        case mxUINT8_CLASS:
            for(vtkIdType i = 0; i < numPoints; i++) for(vtkIdType j = 0; j < 3; j++) ((uint8_T*)mxGetData(points))[numPoints*j+i] = inPointSet->GetPoint(i)[j];
            break;
        case mxINT16_CLASS:
            for(vtkIdType i = 0; i < numPoints; i++) for(vtkIdType j = 0; j < 3; j++) ((int16_T*)mxGetData(points))[numPoints*j+i] = inPointSet->GetPoint(i)[j];
            break;
        case mxUINT16_CLASS:
            for(vtkIdType i = 0; i < numPoints; i++) for(vtkIdType j = 0; j < 3; j++) ((uint16_T*)mxGetData(points))[numPoints*j+i] = inPointSet->GetPoint(i)[j];
            break;
        case mxINT32_CLASS:
            for(vtkIdType i = 0; i < numPoints; i++) for(vtkIdType j = 0; j < 3; j++) ((int32_T*)mxGetData(points))[numPoints*j+i] = inPointSet->GetPoint(i)[j];
            break;
        case mxUINT32_CLASS:
            for(vtkIdType i = 0; i < numPoints; i++) for(vtkIdType j = 0; j < 3; j++) ((uint32_T*)mxGetData(points))[numPoints*j+i] = inPointSet->GetPoint(i)[j];
            break;
        case mxINT64_CLASS:
            for(vtkIdType i = 0; i < numPoints; i++) for(vtkIdType j = 0; j < 3; j++) ((int64_T*)mxGetData(points))[numPoints*j+i] = inPointSet->GetPoint(i)[j];
            break;
        case mxUINT64_CLASS:
            for(vtkIdType i = 0; i < numPoints; i++) for(vtkIdType j = 0; j < 3; j++) ((uint64_T*)mxGetData(points))[numPoints*j+i] = inPointSet->GetPoint(i)[j];
            break;
        case mxSINGLE_CLASS:
            for(vtkIdType i = 0; i < numPoints; i++) for(vtkIdType j = 0; j < 3; j++) ((float*)mxGetData(points))[numPoints*j+i] = inPointSet->GetPoint(i)[j];
            break;
        case mxDOUBLE_CLASS:
            for(vtkIdType i = 0; i < numPoints; i++) for(vtkIdType j = 0; j < 3; j++) ((double*)mxGetData(points))[numPoints*j+i] = inPointSet->GetPoint(i)[j];
            break;
        default:
            mexErrMsgTxt("Unknown Matlab data type of points.");
    }
    
    printVerbose("Processing cells...\n", verbose);
    
    for(vtkIdType i = 0; i < numCells; i++)
    {
        vtkCell* cell = inPointSet->GetCell(i);
        
        vtkIdType numPoints = cell->GetNumberOfPoints();
        for(int j = 0; j < numPoints; j++)
            ((int32_t*)mxGetData(cells))[numCells*j+i] = (int32_t)cell->GetPointId(j)+1;
        
        ((uint8_t*)mxGetData(cellTypes))[i] = cell->GetCellType();
    }
    
    ///// Process point data /////
    
    printVerbose("Processing point data...\n", verbose);
    
    vtkPointData* pointData = inPointSet->GetPointData();
    int numPointDataArrays = pointData->GetNumberOfArrays();
    
    const char** pointDataArrayNames = new const char*[numPointDataArrays];
    for(int i = 0; i < numPointDataArrays; i++)
        pointDataArrayNames[i] = pointData->GetArrayName(i);
    
    mxArray* pointDataStruct = mxCreateStructMatrix(1, 1, numPointDataArrays, pointDataArrayNames);
    for(int i = 0; i < numPointDataArrays; i++)
    {
        mxArray* mxPointDataArray = vtkDataArrayToMxArray(pointData->GetArray(i));
        mxSetFieldByNumber(pointDataStruct, 0, i, mxPointDataArray);
    }
    
    delete[] pointDataArrayNames;
    
    ///// Process cell data /////
    
    printVerbose("Processing cell data...\n", verbose);
    
    vtkCellData* cellData = inPointSet->GetCellData();
    int numCellDataArrays = cellData->GetNumberOfArrays();
    
    const char** cellDataArrayNames = new const char*[numCellDataArrays];
    for(int i = 0; i < numCellDataArrays; i++)
        cellDataArrayNames[i] = cellData->GetArrayName(i);
    
    mxArray* cellDataStruct = mxCreateStructMatrix(1, 1, numCellDataArrays, cellDataArrayNames);
    for(int i = 0; i < numCellDataArrays; i++)
    {
        mxArray* mxCellDataArray = vtkDataArrayToMxArray(cellData->GetArray(i));
        mxSetFieldByNumber(cellDataStruct, 0, i, mxCellDataArray);
    }
    
    delete[] cellDataArrayNames;
    
    ///// Process field data /////
    
    printVerbose("Processing field data...\n", verbose);
    
    vtkFieldData* fieldData = inPointSet->GetFieldData();
    int numFieldDataArrays = fieldData->GetNumberOfArrays();

    const char** fieldDataArrayNames = new const char*[numFieldDataArrays];
    for(int i = 0; i < numFieldDataArrays; i++)
        fieldDataArrayNames[i] = fieldData->GetArrayName(i);
    
    mxArray* fieldDataStruct = mxCreateStructMatrix(1, 1, numFieldDataArrays, fieldDataArrayNames);
    for(int i = 0; i < numFieldDataArrays; i++)
    {
        mxArray* mxFieldDataArray = vtkDataArrayToMxArray(fieldData->GetArray(i));
        mxSetFieldByNumber(fieldDataStruct, 0, i, mxFieldDataArray);
    }
    
    delete[] fieldDataArrayNames;
    
    ///// Assemble output struct /////
    
    printVerbose("Assembling output struct...\n", verbose);
    
    const char* fieldnames[6] = {"points", "cells", "cellTypes", "pointData", "cellData", "fieldData"};
    mxArray* outStruct = mxCreateStructMatrix(1, 1, 6, fieldnames);
    mxSetField(outStruct, 0, "points", points);
    mxSetField(outStruct, 0, "cells", cells);
    mxSetField(outStruct, 0, "cellTypes", cellTypes);
    mxSetField(outStruct, 0, "pointData", pointDataStruct);
    mxSetField(outStruct, 0, "cellData", cellDataStruct);
    mxSetField(outStruct, 0, "fieldData", fieldDataStruct);
    
    return outStruct;
}
