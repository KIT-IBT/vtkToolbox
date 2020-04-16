% Creates a point and cell data array 'RegionId' that contains different
% integers for disconnected regions.
% RegionIds start with 0 and do NOT have a particular order.
%
% Syntax:
% outStruct = vtkConnectivityFilter(struct inStruct)
%
% Written in 2019 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkConnectivityFilter(inStruct) %#ok
