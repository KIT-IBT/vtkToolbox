% Convenience function to create a VTK struct from an array of points and
% cells (the latter is optional).
%
% Syntax:
% outStruct = vtkCreateStruct(numeric points, numeric cells)
%
% points: [numPoints x numDim], numDim <= 3
% cells:  [numCells x numPointsPerCell] (optional)
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkCreateStruct(points, cells)

if size(points,2) > 3
end

outStruct.points = points;

if nargin > 1
    if any(isnan(cells(:)))
        error('NaN(s) found in cells.');
    end
    if any(cells(:) < 1)
        error('Entries of cells must be > 0.');
    end
    if any(cells(:) > size(points,1))
        error('Entries of cells must be <= size(points,1).');
    end
    
    outStruct.cells = int32(cells);
    
    if sum(abs(double(outStruct.cells(:))-double(cells(:))))
        error('cells could not be converted to int32.');
    end
    
    % see https://vtk.org/doc/nightly/html/vtkCellType_8h_source.html
    switch size(cells,2)
        case 0
            cellTypeNum = 0;
        case 1
            cellTypeNum = 1;
        case 2
            cellTypeNum = 3;
        case 3
            cellTypeNum = 5;
        case 4
            cellTypeNum = 10;
        case 6
            cellTypeNum = 22;
        case 10
            cellTypeNum = 24;
        otherwise
            error('Unable to determine appropriate cell type from size(cells,2).');
    end
    
    outStruct.cellTypes = repmat(uint8(cellTypeNum), size(cells,1), 1);
else
    outStruct.cells = int32([]);
    outStruct.cellTypes = uint8([]);
end

end