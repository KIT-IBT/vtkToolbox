
#ifndef __vtkOFFWriter_h
#define __vtkOFFWriter_h

#include <vtkWriter.h>

class vtkPolyData;

class vtkOFFWriter : public vtkWriter
{
public:
    static vtkOFFWriter *New();
    vtkTypeMacro(vtkOFFWriter,vtkWriter)
    void PrintSelf(ostream& os, vtkIndent indent) override;
    
    vtkGetMacro(DecimalPrecision, int)
    vtkSetMacro(DecimalPrecision, int)
    
    vtkSetStringMacro(FileName)
    vtkGetStringMacro(FileName)
    
    vtkPolyData* GetInput();
    vtkPolyData* GetInput(int port);
    
protected:
    vtkOFFWriter();
    ~vtkOFFWriter();
    
    void WriteData() override;
    int FillInputPortInformation(int port, vtkInformation *info) override;
    
    int DecimalPrecision;
    char *FileName;
    
private:
    vtkOFFWriter(const vtkOFFWriter&) = delete;
    void operator=(const vtkOFFWriter&) = delete;
};

#endif
