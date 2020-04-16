% Maps all cell data to point data.
%
% Syntax:
% outStruct = vtkCellDataToPointData(struct inStruct, (bool passCellData))
%
% Defaults:
% passCellData = false
%
% Written in 2019 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkCellDataToPointData(inStruct, passCellData) %#ok
