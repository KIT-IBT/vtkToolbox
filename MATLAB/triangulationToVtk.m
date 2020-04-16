function vtkStruct = triangulationToVtk(TR)

vtkStruct.points = TR.Points;
vtkStruct.cells = int32(TR.ConnectivityList);

switch size(vtkStruct.cells,2)
    case 3
        vtkStruct.cellTypes = repmat(uint8(5), size(vtkStruct.cells,1), 1);
    case 4
        vtkStruct.cellTypes = repmat(uint8(10), size(vtkStruct.cells,1), 1);
    otherwise
        error('Unsupported number of points per cell.');
end

end