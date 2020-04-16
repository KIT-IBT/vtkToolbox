% Cleans vtkPolyData by merging points closer than a tolerance distance.
%
% Syntax:
% outStruct = vtkCleanPolyData(struct inStruct, (double tolerance), (bool toleranceIsAbsolute))
%
% Defaults:
% tolerance = 0
% toleranceIsAbsolute = false
%
% Written in 2018 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkCleanPolyData(inStruct, tolerance, toleranceIsAbsolute) %#ok
