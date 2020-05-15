% Extrudes polygonal data along a vector.
% The amount of extrusion is determined by scaleFactor*norm(vector).
%
% Syntax:
% outStruct = vtkLinearExtrusionFilter(struct inStruct, double vector, double scaleFactor)
%
% vector:      [3 x 1] vector defining the direction of extrusion
% scaleFactor: [1 x 1] factor defining the amount of extrusion
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkLinearExtrusionFilter(inStruct, vector, scaleFactor) %#ok
