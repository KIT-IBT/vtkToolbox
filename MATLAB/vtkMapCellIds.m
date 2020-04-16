% Maps cell ids from a source mesh onto a target mesh.
% Ids start from 1 (Matlab convention, not VTK/C++).
%
% Syntax:
% cellIds = vtkMapCellIds(struct sourceStruct, struct targetStruct)
%
% Written in 2019 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function cellIds = vtkMapCellIds(sourceStruct, targetStruct)

if size(sourceStruct.cells,1)
    sourceStruct.cellData.MappedIds = int32(1:size(sourceStruct.cells,1))';
end
targetStruct = vtkArrayMapperNearestNeighbor(sourceStruct, targetStruct);
cellIds = targetStruct.cellData.MappedIds;

end
