% Maps point ids from a source mesh onto a target mesh.
% Ids start from 1 (Matlab convention, not VTK/C++).
%
% Syntax:
% pointIds = vtkMapPointIds(struct sourceStruct, struct targetStruct, (bool mapUsingVtk))
%
% Written in 2019 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function pointIds = vtkMapPointIds(sourceStruct, targetStruct, mapUsingVtk)

if nargin < 3
    mapUsingVtk = false;
end

if mapUsingVtk
    if size(sourceStruct.points,1)
        sourceStruct.pointData.MappedIds = int32(1:size(sourceStruct.points,1))';
    end
    targetStruct = vtkArrayMapperNearestNeighbor(sourceStruct, targetStruct);
    pointIds = targetStruct.pointData.MappedIds;
else
    pointIds = int32(knnsearch(sourceStruct.points, targetStruct.points, 'NSMethod','kdtree'));
end

end
