% Applies the vtkFeatureEdges filter.
%
% Syntax:
% outStruct = vtkFeatureEdges(struct inStruct, 
%                             (bool boundaryEdgesOn),
%                             (bool featureEdgesOn),
%                             (bool nonManifoldEdgesOn),
%                             (bool manifoldEdgesOn),
%                             (bool coloringOn),
%                             (double featureAngle))
%
% Defaults:
% boundaryEdgesOn    = true
% featureEdgesOn     = true
% nonManifoldEdgesOn = true
% manifoldEdgesOn    = false
% coloringOn         = true
% featureAngle       = 30.0
%
% Written in 2019 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkFeatureEdges(inStruct, boundaryEdgesOn, featureEdgesOn, nonManifoldEdgesOn, manifoldEdgesOn, coloringOn, featureAngle) %#ok
