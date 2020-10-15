% Computes a matrix for linear interpolation of point data between two
% triangle meshes.
%
% Syntax:
% [M, baryCellIds, baryCoords, intersectionStruct, targetStruct] = vtkBarycentricInterpMat(struct sourceStruct, struct targetStruct, double normalLength)
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function [M, baryCellIds, baryCoords, intersectionStruct, targetStruct] = vtkBarycentricInterpMat(sourceStruct, targetStruct, normalLength)

[baryCellIds, baryCoords, intersectionStruct, targetStruct]= vtkBarycentricCoords(sourceStruct, targetStruct, normalLength);
notFound = baryCellIds < 1;
baryCellIds(notFound) = 1;
baryPointIds = double(sourceStruct.cells(baryCellIds,:));

m = size(baryCoords,1);
n = size(sourceStruct.points,1);
i = reshape(repmat((1:m)',1,3),[],1);
j = baryPointIds(:);
v = baryCoords(:);
v(notFound) = nan;
M = sparse(i,j,v,m,n);

end