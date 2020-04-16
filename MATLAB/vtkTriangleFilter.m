% Converts input polygons and strips to triangles.
% Polylines are converted to line segments.
%
% Syntax:
% outStruct = vtkTriangleFilter(struct inStruct)
%
% Written in 2019 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkTriangleFilter(inStruct) %#ok
