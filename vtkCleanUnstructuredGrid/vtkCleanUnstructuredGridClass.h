/*=========================================================================
 
 Program:   ParaView v.5.6.0
 Module:    vtkCleanUnstructuredGridClass.h
 
 Copyright (c) Kitware, Inc.
 All rights reserved.
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.
 
 =========================================================================*/

/**
 * @class   vtkCleanUnstructuredGrid
 * @brief   merge points that are closer than a given tolerance
 *
 *
 * vtkCleanUnstructuredGrid is a filter that takes unstructured grid data as
 * input and generates unstructured grid data as output.
 * vtkCleanUnstructuredGrid can merge duplicate or close points using the
 * vtkPointLocator object to merge points.
 *
 * @sa
 * vtkCleanUnstructuredGrid
 */

#ifndef vtkCleanUnstructuredGridClass_h
#define vtkCleanUnstructuredGridClass_h

#include "vtkUnstructuredGridAlgorithm.h"

class vtkPointLocator;

class vtkCleanUnstructuredGrid : public vtkUnstructuredGridAlgorithm
{
public:
    static vtkCleanUnstructuredGrid* New();
    
    vtkTypeMacro(vtkCleanUnstructuredGrid, vtkUnstructuredGridAlgorithm);
    
    void SetTolerance(double tolerance);
    void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;
    
protected:
    vtkCleanUnstructuredGrid();
    ~vtkCleanUnstructuredGrid() override;
    
    vtkPointLocator* Locator;
    
    int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) VTK_OVERRIDE;
    int FillInputPortInformation(int port, vtkInformation* info) VTK_OVERRIDE;
    
private:
    vtkCleanUnstructuredGrid(const vtkCleanUnstructuredGrid&) = delete;
    void operator=(const vtkCleanUnstructuredGrid&) = delete;
};
#endif
