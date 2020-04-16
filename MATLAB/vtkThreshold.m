% Applies a threshold on a point or cell data array.
% The input can be vtkPolyData or vtkUnstructuredGrid.
% The output is always a vtkUnstructuredGrid.
% Use vtkDataSetSurfaceFilter afterwards if you need a vtkPolyData.
%
% Syntax:
% outStruct = vtkThreshold(struct inStruct, char cellsOrPoints, char arrayName, double thresholds)
%
% cellsOrPoints: 'cells' or 'points'
% arrayName:     the name of the data array
% thresholds:    [lower upper] (closed interval, including bounds)
%
% Written in 2019 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkThreshold(inStruct, cellsOrPoints, arrayName, thresholds) %#ok
