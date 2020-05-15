% Performs loop subdivision of vtkPolyData.
%
% Syntax:
% outStruct = vtkLoopSubdivisionFilter(struct inStruct, int numberOfSubdivisions)
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkLoopSubdivisionFilter(inStruct, numberOfSubdivisions) %#ok
