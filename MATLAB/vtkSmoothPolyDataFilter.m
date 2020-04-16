% Performs Laplacian smoothing.
%
% Syntax:
% outStruct = vtkSmoothPolyDataFilter(struct inStruct, int numberOfIterations)
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkSmoothPolyDataFilter(inStruct, numberOfIterations) %#ok
