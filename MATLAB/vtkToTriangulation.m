function TR = vtkToTriangulation(vtkStruct)

TR = triangulation(double(vtkStruct.cells), double(vtkStruct.points));

end