% Maps all point data to cell data.
%
% Syntax:
% outStruct = vtkPointDataToCellData(struct inStruct, (bool passPointData))
%
% Defaults:
% passPointData = false
%
% Written in 2019 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkPointDataToCellData(inStruct, passPointData) %#ok
