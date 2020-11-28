% Computes the center of area (center of mass of a surface, assuming 
% uniform density) of a triangle mesh.
% Relies on gptoolbox by Alec Jacobson.
%
% Syntax:
% [centerOfArea,area,a] = vtkCenterOfArea(inStruct)
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function [centerOfArea,area,a] = vtkCenterOfArea(inStruct)

P = double(inStruct.points);
C = inStruct.cells;

if size(C,2) ~=3
    error('This function can only be applied to triangle meshes.');
end

centroids = squeeze(mean(reshape(P(C,:),[],size(C,2),size(P,2)),2));
a = doublearea(P,C)/2;
area = sum(a);

centerOfArea = sum(repmat(a,1,3) .* centroids, 1) ./ area;

end