% Cleans vtkUnstructuredGrid by merging points closer than a tolerance
% distance. This is especially useful to get rid of duplicate points after
% appending adjacent geometries.
%
% Syntax:
% outStruct = vtkCleanUnstructuredGrid(struct inStruct, (double tolerance))
%
% Defaults:
% tolerance = 2e-4 (at least twice the precision of most ascii formats)
%
% Written in 2019 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkCleanUnstructuredGrid(inStruct, tolerance) %#ok
