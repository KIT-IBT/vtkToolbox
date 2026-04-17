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
#include <algorithm>
#include <cctype>
#include <memory>
#include <string>


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

namespace {

std::string getFilenameFromMxArray(const mxArray* arr, const std::string& syntax)
{
    if (arr == nullptr) {
        mexErrMsgIdAndTxt("vtkToolbox:vtkRead:InvalidInput",
                          "Filename input is null. Syntax: %s", syntax.c_str());
    }

    // Accept classic char vectors / char arrays
    if (mxIsChar(arr)) {
        char* raw = mxArrayToString(arr);
        if (raw == nullptr) {
            mexErrMsgIdAndTxt("vtkToolbox:vtkRead:ConversionFailed",
                              "Failed to convert filename char array to C string.");
        }

        std::string out(raw);
        mxFree(raw);

        if (out.empty()) {
            mexErrMsgIdAndTxt("vtkToolbox:vtkRead:EmptyFilename",
                              "Filename must not be empty.");
        }
        return out;
    }

    // Accept MATLAB string scalar by converting through MATLAB itself
    if (mxIsClass(arr, "string")) {
        mxArray* rhs[1] = { const_cast<mxArray*>(arr) };
        mxArray* lhs[1] = { nullptr };

        if (mexCallMATLAB(1, lhs, 1, rhs, "char") != 0 || lhs[0] == nullptr) {
            mexErrMsgIdAndTxt("vtkToolbox:vtkRead:ConversionFailed",
                              "Failed to convert MATLAB string input to char.");
        }

        char* raw = mxArrayToString(lhs[0]);
        mxDestroyArray(lhs[0]);

        if (raw == nullptr) {
            mexErrMsgIdAndTxt("vtkToolbox:vtkRead:ConversionFailed",
                              "Failed to convert MATLAB string filename to C string.");
        }

        std::string out(raw);
        mxFree(raw);

        if (out.empty()) {
            mexErrMsgIdAndTxt("vtkToolbox:vtkRead:EmptyFilename",
                              "Filename must not be empty.");
        }
        return out;
    }

    mexErrMsgIdAndTxt("vtkToolbox:vtkRead:InvalidFilenameType",
                      "First input must be a character array or a string scalar.");
    return std::string(); // unreachable
}

} // namespace

/* MATLAB entry function
 * nlhs/nrhs contain the number of left/right-hand-side arguments to this function
 * plhs/prhs are arrays of pointers to the arguments in MATLAB data format */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    const std::string syntax = "outStruct = vtkRead(filename, (verbose))";

    if (nrhs < 1)
        mexErrMsgIdAndTxt("vtkToolbox:vtkRead:NotEnoughInputs",
                          "Not enough input arguments. Syntax: %s", syntax.c_str());
    if (nrhs > 2)
        mexErrMsgIdAndTxt("vtkToolbox:vtkRead:TooManyInputs",
                          "Too many input arguments. Syntax: %s", syntax.c_str());
    if (nlhs > 1)
        mexErrMsgIdAndTxt("vtkToolbox:vtkRead:TooManyOutputs",
                          "Too many output arguments. Syntax: %s", syntax.c_str());

    bool verbose = false;
    if (nrhs == 2) {
        if (!mxIsLogicalScalar(prhs[1]) && !mxIsDouble(prhs[1])) {
            mexErrMsgIdAndTxt("vtkToolbox:vtkRead:InvalidVerbose",
                              "Second input 'verbose' must be logical or numeric scalar.");
        }
        verbose = mxGetScalar(prhs[1]) != 0.0;
    }

    printVerbose("Reading file...\n", verbose);

    const std::string path = getFilenameFromMxArray(prhs[0], syntax);

    const std::size_t pos = path.find_last_of('.');
    if (pos == std::string::npos || pos + 1 >= path.size()) {
        mexErrMsgIdAndTxt("vtkToolbox:vtkRead:MissingExtension",
                          "Filename must include a supported file extension.");
    }

    std::string extension = path.substr(pos + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    vtkSmartPointer<vtkPointSet> pointSet;

    if (extension == "vtk") {
        vtkSmartPointer<vtkDataSetReader> reader = vtkSmartPointer<vtkDataSetReader>::New();
        reader->SetFileName(path.c_str());
        if (!reader->OpenVTKFile()) {
            mexErrMsgIdAndTxt("vtkToolbox:vtkRead:OpenFailed",
                              "File cannot be opened by vtkDataSetReader. Does it exist?");
        }
        reader->Update();

        if (reader->IsFilePolyData())
            pointSet = reader->GetPolyDataOutput();
        else if (reader->IsFileUnstructuredGrid())
            pointSet = reader->GetUnstructuredGridOutput();
        else
            mexErrMsgIdAndTxt("vtkToolbox:vtkRead:UnsupportedDataFormat",
                              "VTK internal data format is not supported.");
    }
    else if (extension == "vtp") {
        vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
        if (!reader->CanReadFile(path.c_str())) {
            mexErrMsgIdAndTxt("vtkToolbox:vtkRead:ReadFailed",
                              "File cannot be read by vtkXMLPolyDataReader. Does it exist?");
        }
        reader->SetFileName(path.c_str());
        reader->Update();
        pointSet = reader->GetOutput();
    }
    else if (extension == "vtu") {
        vtkSmartPointer<vtkXMLUnstructuredGridReader> reader =
            vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
        if (!reader->CanReadFile(path.c_str())) {
            mexErrMsgIdAndTxt("vtkToolbox:vtkRead:ReadFailed",
                              "File cannot be read by vtkXMLUnstructuredGridReader. Does it exist?");
        }
        reader->SetFileName(path.c_str());
        reader->Update();
        pointSet = reader->GetOutput();
    }
    else if (extension == "ply") {
        vtkSmartPointer<vtkPLYReader> reader = vtkSmartPointer<vtkPLYReader>::New();
        if (!reader->CanReadFile(path.c_str())) {
            mexErrMsgIdAndTxt("vtkToolbox:vtkRead:ReadFailed",
                              "File cannot be read by vtkPLYReader. Does it exist?");
        }
        reader->SetFileName(path.c_str());
        reader->Update();
        pointSet = reader->GetOutput();
    }
    else if (extension == "stl") {
        vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
        reader->SetFileName(path.c_str());
        reader->Update();
        pointSet = reader->GetOutput();
    }
    else if (extension == "obj") {
        vtkSmartPointer<vtkOBJReader> reader = vtkSmartPointer<vtkOBJReader>::New();
        reader->SetFileName(path.c_str());
        reader->Update();
        pointSet = reader->GetOutput();
    }
    else {
        mexErrMsgIdAndTxt("vtkToolbox:vtkRead:UnknownExtension",
                          "Unknown extension \"%s\".", extension.c_str());
    }

    plhs[0] = vtkToStruct(pointSet, verbose);
}