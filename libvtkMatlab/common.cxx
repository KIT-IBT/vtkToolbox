#include "common.h"

#include "vtkCellTypes.h"
#include "vtkFieldData.h"
#include "vtkCellData.h"
#include "vtkPointData.h"

void printVerbose(const char* msg, bool verbose)
{
    if(verbose)
    {
        mexPrintf(msg);
        mexEvalString("drawnow;");
    }
}

int getNumberOfCellPoints(uint8_t cellType)
{
    switch(cellType)
    {
        case VTK_VERTEX:
            return(1);
        case VTK_LINE:
            return(2);
        case VTK_TRIANGLE:
            return(3);
        case VTK_PIXEL:
            return(4);
        case VTK_QUAD:
            return(4);
        case VTK_TETRA:
            return(4);
        case VTK_VOXEL:
            return(8);
        case VTK_HEXAHEDRON:
            return(8);
        case VTK_WEDGE:
            return(6);
        case VTK_PYRAMID:
            return(5);
        case VTK_PENTAGONAL_PRISM:
            return(10);
        case VTK_HEXAGONAL_PRISM:
            return(12);
        case VTK_QUADRATIC_EDGE:
            return(3);
        case VTK_QUADRATIC_TRIANGLE:
            return(6);
        case VTK_QUADRATIC_QUAD:
            return(8);
        case VTK_QUADRATIC_TETRA:
            return(10);
        case VTK_QUADRATIC_HEXAHEDRON:
            return(20);
        case VTK_QUADRATIC_WEDGE:
            return(15);
        case VTK_QUADRATIC_PYRAMID:
            return(13);
        default:
            mexErrMsgTxt(("Cell type " + std::string(vtkCellTypes::GetClassNameFromTypeId(cellType)) + " is not supported.").c_str());
    }
    return(-1);
}

vtkDataArray* getDataArray(vtkPointSet* pointSet, char* dataAssociation, char* arrayName)
{
    vtkFieldData* data = 0;
    if(strcmp(dataAssociation, "cells") == 0)
        data = pointSet->GetCellData();
    else if(strcmp(dataAssociation, "points") == 0)
        data = pointSet->GetPointData();
    else if(strcmp(dataAssociation, "field") == 0)
        data = pointSet->GetFieldData();
    else
        return 0;
    
    for(int i = 0; i < data->GetNumberOfArrays(); i++)
    {
        if(strcmp(data->GetArrayName(i), arrayName) == 0)
            return data->GetArray(i);
    }
    return 0;
}
