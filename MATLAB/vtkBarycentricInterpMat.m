% Computes a matrix for linear interpolation of point data between two
% triangle meshes.
%
% Syntax:
% M = vtkBarycentricInterpMat(struct sourceStruct, struct targetStruct, double normalLength)
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function M = vtkBarycentricInterpMat(sourceStruct, targetStruct, normalLength)

[baryCellIds,baryCoords]= vtkBarycentricCoords(sourceStruct, targetStruct, normalLength);
baryPointIds = double(sourceStruct.cells(baryCellIds,:));

m = size(baryCoords,1);
n = size(sourceStruct.points,1);
i = reshape(repmat((1:m)',1,3),[],1);
j = baryPointIds(:);
v = baryCoords(:);
M = sparse(i,j,v,m,n);

end