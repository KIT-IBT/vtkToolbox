% Performs boolean operations on vtkPolyData.
%
% Syntax:
% outStruct = vtkBooleanOperationPolyDataFilter(struct inStruct1, struct inStruct2, (char operation))
%
% operation: 'union', 'intersection' or 'difference'
%
% Defaults:
% operation = 'union'
%
% Written in 2018 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkBooleanOperationPolyDataFilter(inStruct1, inStruct2, operation) %#ok
