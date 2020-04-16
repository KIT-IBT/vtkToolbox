% Projects electrode positions onto the body surface and moves the closest
% node of the body surface mesh to the projected electrode position.
%
% Syntax:
% outStruct = vtkAlignNodesWithElectrodes(struct bodyStruct, struct electrodesStruct)
%
% Written in 2019 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkAlignNodesWithElectrodes(bodyStruct, electrodesStruct) %#ok
