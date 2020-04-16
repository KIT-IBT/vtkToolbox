/*****************************************************************
 vtkRead
 MATLAB extension to read VTK files (legacy: vtk or XML: vtp, vtu)
 Also supported are: ply, stl, obj, (off not yet)
 -----------------------------------------------------------------
 Before first usage:
 Build using the script vtk_mex_build.m
 
 Usage inside MATLAB:
 outStruct = vtkRead(char filename, [bool verbose]);
 
 verbose is false on default
 -----------------------------------------------------------------
 Author: Steffen Schuler
         Institute of Biomedical Engineering
         Karlsruhe Institute of Technology
         www.ibt.kit.edu
 
 Created:     June 2017
 Last edited: January 2018
 *****************************************************************/

#include "vtkToStruct.h"

// legacy format reader
#include <vtkDataSetReader.h>

// XML format readers
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLUnstructuredGridReader.h>

// readers for other common polygon mesh formats
#include <vtkPLYReader.h>
#include <vtkSTLReader.h>
#include <vtkOBJReader.h>
//#include "additionalReaders/vtkOFFReader.h"

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    const std::string syntax = "outStruct = vtkRead(filename, (verbose))";
    if(nrhs < 1)
        mexErrMsgTxt(("Not enough input arguments. Syntax: " + syntax).c_str());
    if(nrhs > 2)
        mexErrMsgTxt(("Too many input arguments. Syntax: " + syntax).c_str());
    if(nlhs > 1)
        mexErrMsgTxt(("Too many output arguments. Syntax: " + syntax).c_str());
    
    bool verbose = false;
    if(nrhs == 2)
        verbose = mxGetScalar(prhs[1]);
    
    ///// Read file /////
    
    printVerbose("Reading file...\n", verbose);
    
    std::string path(mxArrayToString(prhs[0]));
    long pos = path.find_last_of(".");
    std::string extension = path.substr(pos+1, path.size()-pos);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    vtkSmartPointer<vtkPointSet> pointSet;
    
    if(extension == "vtk")
    {
        vtkSmartPointer<vtkDataSetReader> reader = vtkSmartPointer<vtkDataSetReader>::New();
        reader->SetFileName(path.c_str());
        if(!reader->OpenVTKFile())
            mexErrMsgTxt("File cannot be opened by vtkDataSetReader. Does it exist?");
        reader->Update();
        
        if(reader->IsFilePolyData())
            pointSet = reader->GetPolyDataOutput();
        else if(reader->IsFileUnstructuredGrid())
            pointSet = reader->GetUnstructuredGridOutput();
        else
            mexErrMsgTxt("VTK internal dataformat is not supported.");
    }
    else if(extension == "vtp")
    {
        vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
        if(!reader->CanReadFile(path.c_str()))
            mexErrMsgTxt("File cannot be read by vtkXMLPolyDataReader. Does it exist?");
        reader->SetFileName(path.c_str());
        reader->Update();
        pointSet = reader->GetOutput();
    }
    else if(extension == "vtu")
    {
        vtkSmartPointer<vtkXMLUnstructuredGridReader> reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
        if(!reader->CanReadFile(path.c_str()))
            mexErrMsgTxt("File cannot be read by vtkXMLUnstructuredGridReader. Does it exist?");
        reader->SetFileName(path.c_str());
        reader->Update();
        pointSet = reader->GetOutput();
    }
    else if(extension == "ply")
    {
        vtkSmartPointer<vtkPLYReader> reader = vtkSmartPointer<vtkPLYReader>::New();
        if(!reader->CanReadFile(path.c_str()))
            mexErrMsgTxt("File cannot be read by vtkPLYReader. Does it exist?");
        reader->SetFileName(path.c_str());
        reader->Update();
        pointSet = reader->GetOutput();
    }
    else if(extension == "stl")
    {
        vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
        reader->SetFileName(path.c_str());
        reader->Update();
        pointSet = reader->GetOutput();
    }
    else if(extension == "obj")
    {
        vtkSmartPointer<vtkOBJReader> reader = vtkSmartPointer<vtkOBJReader>::New();
        reader->SetFileName(path.c_str());
        reader->Update();
        pointSet = reader->GetOutput();
    }
//    else if(extension == "off")
//    {
//        vtkSmartPointer<vtkOFFReader> reader = vtkSmartPointer<vtkOFFReader>::New();
//        reader->SetFileName(path.c_str());
//        reader->Update();
//        pointSet = reader->GetOutput();
//    }
    else
        mexErrMsgTxt((std::string("Unknown extension \"") + extension + "\".").c_str());
    
    ///// Convert vtkPointSet into MATLAB struct /////
    
    plhs[0] = vtkToStruct(pointSet, verbose);
}
