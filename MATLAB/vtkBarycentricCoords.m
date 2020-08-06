% For each point in the target mesh, this function computes the barycentric
% coordinates of the point at which the target normal intersects the source
% mesh. This can be used to linearly interpolate data between two
% non-conforming triangle meshes, see vtkBarycentricInterpMat.
%
% Syntax:
% [sourceCellIds, baryCoords, intersectionStruct, targetStruct] = vtkBarycentricCoords(struct sourceStruct, struct targetStruct, double normalLength)
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function [sourceCellIds, baryCoords, intersectionStruct, targetStruct] = vtkBarycentricCoords(sourceStruct, targetStruct, normalLength) %#ok