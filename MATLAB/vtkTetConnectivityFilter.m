% Assigns regions to a tetrahedral mesh based on face connectivities.
% The result is stored in a cell data array 'RegionId'.
% This function differs from vtkConnectivityFilter: Here, only tets sharing
% an entire face are considered neighbors, while vtkConnectivityFilter
% considers tets sharing at least one node as neighbors.
%
% Syntax:
% outStruct = vtkTetConnectivityFilter(inStruct)
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkTetConnectivityFilter(inStruct)

if size(inStruct.cells,2) ~= 4
    error('This function can only be used with tetrahedral meshes.');
end

tetras = inStruct.cells;
comb = nchoosek([1 2 3 4],3);
comb = reshape((comb + repmat((0:4:12)',1,3))', [], numel(comb));
faces = repmat(tetras, 1, 4);
faces = reshape(faces(:,comb)', 3, [])';
faces = sort(faces,2); %#ok

[sortFaces,sortInd] = sortrows(faces);
d = ~logical(sum(abs(diff(sortFaces,1,1)),2));
faceNeighPairs = [sortInd([d; false]) sortInd([false; d])];
tetraNeighPairs = floor((faceNeighPairs-1)/4)+1;
tetraNeighPairs = sortrows(tetraNeighPairs); % this line is optional

tetraNeighbors = NaN(size(tetras,1),4);
for i = 1:size(tetraNeighPairs,1)
    tetraNeighbors(tetraNeighPairs(i,1), find(isnan(tetraNeighbors(tetraNeighPairs(i,1),:)),1)) = tetraNeighPairs(i,2);
    tetraNeighbors(tetraNeighPairs(i,2), find(isnan(tetraNeighbors(tetraNeighPairs(i,2),:)),1)) = tetraNeighPairs(i,1);
end

regionId = zeros(size(tetras,1), 1, 'int64');
for currentRegionId = 1:size(tetras,1)
    list = find(regionId==0,1);
    if isempty(list)
        break;
    end
    for i = 1:size(tetras,1)
        regionId(list) = currentRegionId;
        list = setdiff(tetraNeighbors(list,:), find(regionId));
        list(isnan(list)) = [];
        if isempty(list)
            break;
        end
    end
end

outStruct = inStruct;
outStruct.cellData.RegionId = regionId;

end