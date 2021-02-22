
#include "vtkOFFWriter.h"

#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkInformation.h>
#include <vtkErrorCode.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkOFFWriter);

vtkOFFWriter::vtkOFFWriter()
{
    std::ofstream fout; // only used to extract the default precision
    this->DecimalPrecision = fout.precision();
    this->FileName = NULL;
}

vtkOFFWriter::~vtkOFFWriter(){}

void vtkOFFWriter::WriteData()
{
    vtkPolyData *input = this->GetInput();
    if(!input)
        return;
    
    if(!this->FileName)
    {
        vtkErrorMacro(<< "No FileName specified! Can't write!");
        this->SetErrorCode(vtkErrorCode::NoFileNameError);
        return;
    }
    
    vtkDebugMacro(<<"Opening vtk file for writing...");
    ostream *outfilep = new std::ofstream(this->FileName, ios::out);
    if(outfilep->fail())
    {
        vtkErrorMacro(<< "Unable to open file: "<< this->FileName);
        this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
        delete outfilep;
        return;
    }
    
    std::ostream& outfile = *outfilep;
    double p[3];
    
    vtkIdType numPoints = input->GetNumberOfPoints();
    vtkIdType numCells = input->GetNumberOfCells();
    
    outfile << "OFF" << std::endl;
    outfile << numPoints << " " << numCells << " 0" << std::endl;
    
    // write points
    for(int i = 0; i < numPoints; i++)
    {
        input->GetPoint(i, p);
        outfile << std::setprecision(this->DecimalPrecision) << p[0] << " " << p[1] << " " << p[2] << std::endl;
    }
    
    // write cells
    vtkIdType npts = 0;
    const vtkIdType *pts = NULL;
    for(int i = 0; i < numCells; i++)
    {
        input->GetCellPoints(i, npts, pts);
        outfile << npts;
        for(int i = 0; i < npts; i++)
            outfile << " " << pts[i];
        outfile << std::endl;
    }
    
    vtkDebugMacro(<<"Closing vtk file\n");
    delete outfilep;
    
    // Delete the file if an error occurred
    if(this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError)
    {
        vtkErrorMacro("Ran out of disk space; deleting file: " << this->FileName);
        std::remove(this->FileName);
    }
}

void vtkOFFWriter::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os, indent);
    os << indent << "DecimalPrecision: " << DecimalPrecision << "\n";
}

int vtkOFFWriter::FillInputPortInformation(int, vtkInformation *info)
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    return 1;
}

vtkPolyData* vtkOFFWriter::GetInput()
{
    return vtkPolyData::SafeDownCast(this->Superclass::GetInput());
}

vtkPolyData* vtkOFFWriter::GetInput(int port)
{
    return vtkPolyData::SafeDownCast(this->Superclass::GetInput(port));
}
