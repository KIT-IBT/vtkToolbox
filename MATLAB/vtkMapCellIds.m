% Maps cell ids from a source mesh onto a target mesh.
% Ids start from 1 (Matlab convention, not VTK/C++).
%
% Syntax:
% cellIds = vtkMapCellIds(struct sourceStruct, struct targetStruct, (bool mapUsingVtk))
%
% Written in 2019 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function cellIds = vtkMapCellIds(sourceStruct, targetStruct, mapUsingVtk)

if nargin < 3
    mapUsingVtk = false;
end

if mapUsingVtk
    if size(sourceStruct.cells,1)
        sourceStruct.cellData.MappedIds = int32(1:size(sourceStruct.cells,1))';
    end
    targetStruct = vtkArrayMapperNearestNeighbor(sourceStruct, targetStruct);
    cellIds = targetStruct.cellData.MappedIds;
else
    sourceCentroids = squeeze(mean(reshape(sourceStruct.points(sourceStruct.cells,:),[],size(sourceStruct.cells,2),size(sourceStruct.points,2)),2));
    targetCentroids = squeeze(mean(reshape(targetStruct.points(targetStruct.cells,:),[],size(targetStruct.cells,2),size(targetStruct.points,2)),2));
    cellIds = int32(knnsearch(sourceCentroids, targetCentroids, 'NSMethod','kdtree'));
end

end
