% Maps point and cell ids from a source mesh onto a target mesh.
% The results are stored as point and cell data arrays 'MappedIds'.
% Ids start from 1 (Matlab convention, not VTK/C++).
%
% Syntax:
% outStruct = vtkMapIds(struct sourceStruct, struct targetStruct)
%
% Written in 2019 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkMapIds(sourceStruct, targetStruct)

if size(sourceStruct.points,1)
    sourceStruct.pointData.MappedIds = int32(1:size(sourceStruct.points,1))';
end
if size(sourceStruct.cells,1)
    sourceStruct.cellData.MappedIds = int32(1:size(sourceStruct.cells,1))';
end
outStruct = vtkArrayMapperNearestNeighbor(sourceStruct, targetStruct);

end
