% Generates isosurfaces/isolines at scalar values of a point or cell data array.
% The input can be vtkPolyData or vtkUnstructuredGrid.
% The output is a vtkPolyData.
%
% Syntax:
% outStruct = vtkContourFilter(struct inStruct, char cellsOrPoints, char arrayName, double values)
%
% cellsOrPoints: 'cells' or 'points'
% arrayName:     the name of the data array
% values:        vector of scalar values (isovalues)
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkContourFilter(inStruct, cellsOrPoints, arrayName, values) %#ok
