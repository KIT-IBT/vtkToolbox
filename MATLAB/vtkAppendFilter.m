% Appends multiple VTK datasets and optionally merges coincidental points
% (merge tolerance = 0.0).
%
% Syntax:
% outStruct = vtkAppendFilter({inStruct1, inStruct2, ...}, (bool mergePoints))
%
% Defaults:
% mergePoints = false
%
% Written in 2019 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkAppendFilter(cellArrayOfStructs, mergePoints) %#ok
