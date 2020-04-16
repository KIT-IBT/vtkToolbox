/**************************************************************************
 vtkWrite
 MATLAB extension to write VTK files (legacy: vtk or XML: vtp, vtu)
 Also supported are: ply, stl, obj, off (only geometry, no data arrays)
 --------------------------------------------------------------------------
 Before first usage:
 Build using the script vtk_mex_build.m
 
 Usage inside MATLAB:
 vtkWrite(struct inStruct, char filename, [bool verbose], [char dataMode]);
 
 inStruct must have the following fields:
     points:    [numPoints x numDim double/single/(u)intN], numDim <= 3
     cells:     [numCells x maxNumPointsPerCell int32]
     cellTypes: [numCells x 1 uint8] (see https://vtk.org/doc/nightly/html/vtkCellType_8h_source.html)
     pointData: [1 x 1 struct of point data arrays] (optional)
     cellData:  [1 x 1 struct of cell data arrays] (optional)
     fieldData: [1 x 1 struct of field data arrays] (optional)
 
 verbose is false on default
 
 dataMode can be chosen from {'ascii', 'binary'} for vtk, ply, stl
                         and {'ascii', 'binary', 'appended'} for vtp, vtu
                         and {'ascii'} for obj, off
 --------------------------------------------------------------------------
 Authors: Steffen Schuler, Nils Daub
          Institute of Biomedical Engineering
          Karlsruhe Institute of Technology
          www.ibt.kit.edu
 
 Created:     November 2017
 Last edited: January 2018
 **************************************************************************/

#include "structToVtk.h"

// legacy format writer
#include <vtkDataSetWriter.h>

// XML format writers
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>

// writers for other common polygon mesh formats
#include <vtkPLYWriter.h>
#include <vtkSTLWriter.h>
#include "additionalWriters/vtkOBJWriter.h"
#include "additionalWriters/vtkOFFWriter.h"

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    const std::string syntax = "vtkWrite(inStruct, filename, (verbose), (dataMode))";
    if(nrhs < 2)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 4)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 0)
        mexErrMsgTxt(("Too many output arguments. There is no return value. Syntax: " + syntax).c_str());
    
    bool verbose = false;
    std::string dataModeString;
    if(nrhs >= 3)
    {
        if(nrhs == 4)
        {
            char* dataModeChar = mxArrayToString(prhs[3]);
            if(dataModeChar == NULL)
                mexErrMsgTxt("Data mode argument could not be read. Must be one of {'ascii', 'binary', 'appended'}.");
            dataModeString = dataModeChar;
        }
        verbose = mxGetScalar(prhs[2]);
    }
    
    ///// Convert MATLAB struct into vtkPointSet /////
    
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPointSet* pointSet = structToVtk(prhs[0], polyData, unstructuredGrid, verbose);
    
    ///// Check file format compatibility /////
    
    printVerbose("Checking file format compatibility...\n", verbose);
    
    std::string path(mxArrayToString(prhs[1]));
    long pos = path.find_last_of(".");
    std::string extension = path.substr(pos+1, path.size()-pos);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if(extension != "vtk" && extension != "vtp" && extension != "vtu" && extension != "ply" && extension != "stl" && extension != "obj" && extension != "off")
        mexErrMsgTxt(("File extension " + extension + " is not supported (must be vtk, vtp, vtu, ply, stl, obj or off).").c_str());
    
    bool requirePolyData = false;
    if(extension != "vtu" && extension != "vtk")
        requirePolyData = true;
    
    bool isPolyData = false;
    if(pointSet->GetDataObjectType() == VTK_POLY_DATA)
        isPolyData = true;
    
    if(requirePolyData && !isPolyData)
        mexErrMsgTxt("vtkPolyData only supports linear surface cells (vertices, lines or triangles). Incompatible cell(s) found.");
    
    ///// Write file /////
    
    printVerbose("Writing file...\n", verbose);
    
    int dataMode = 0;
    if(!dataModeString.empty())
    {
        if(dataModeString == "ascii")
            dataMode = 1;
        else if(dataModeString == "binary")
            dataMode = 2;
        else if(dataModeString == "appended")
        {
            if(extension == "vtp" || extension == "vtu")
                dataMode = 3;
            else
                mexWarnMsgTxt("Data mode 'appended' only supported for vtp and vtu. Using VTK default.");
        }
        else
            mexWarnMsgTxt(("Unknown data mode '" + std::string(dataModeString) + "'. Using VTK default.").c_str());
    }
    
    if(extension == "vtk")
    {
        vtkSmartPointer<vtkDataSetWriter> writer = vtkSmartPointer<vtkDataSetWriter>::New();
        writer->SetInputData(pointSet);
        writer->SetFileName(path.c_str());
        if(dataMode)
            writer->SetFileType(dataMode);
        writer->Write();
    }
    else if(extension == "vtp")
    {
        vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
        writer->SetInputData(pointSet);
        writer->SetFileName(path.c_str());
        if(dataMode)
            writer->SetDataMode(dataMode-1);
        writer->Write();
    }
    else if(extension == "vtu")
    {
        vtkSmartPointer<vtkXMLUnstructuredGridWriter> writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
        writer->SetInputData(pointSet);
        writer->SetFileName(path.c_str());
        if(dataMode)
            writer->SetDataMode(dataMode-1);
        writer->Write();
    }
    else if(extension == "ply")
    {
        vtkSmartPointer<vtkPLYWriter> writer = vtkSmartPointer<vtkPLYWriter>::New();
        writer->SetInputData(pointSet);
        writer->SetFileName(path.c_str());
        if(dataMode)
            writer->SetFileType(dataMode);
        writer->Write();
    }
    else if(extension == "stl")
    {
        vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
        writer->SetInputData(pointSet);
        writer->SetFileName(path.c_str());
        if(dataMode)
            writer->SetFileType(dataMode);
        writer->Write();
    }
    else if(extension == "obj")
    {
        vtkSmartPointer<vtkOBJWriter> writer = vtkSmartPointer<vtkOBJWriter>::New();
        writer->SetInputData(pointSet);
        writer->SetFileName(path.c_str());
        if(dataMode > 1)
            mexWarnMsgTxt("Data mode is always 'ascii' for obj.");
        writer->Write();
    }
    else if(extension == "off")
    {
        vtkSmartPointer<vtkOFFWriter> writer = vtkSmartPointer<vtkOFFWriter>::New();
        writer->SetInputData(pointSet);
        writer->SetFileName(path.c_str());
        if(dataMode > 1)
            mexWarnMsgTxt("Data mode is always 'ascii' for off.");
        writer->Write();
    }
}
