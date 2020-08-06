% Maps data arrays from source points or cells to nearest target points or
% cells.
%
% Syntax:
% outStruct = vtkArrayMapperNearestNeighbor(struct sourceStruct, struct targetStruct)
%
% Written in 2015 by Andreas Wachter
% MEX function by Steffen Schuler, 2019
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkArrayMapperNearestNeighbor(sourceStruct, targetStruct) %#ok
