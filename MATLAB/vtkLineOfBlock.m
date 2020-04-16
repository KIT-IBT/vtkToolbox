% Outputs mesh points within a certain radius around the input points
%
% Syntax:
% outArray = vtkLineOfBlock(struct inStruct, inArray point_array, double radius)
%   pointArray needs to have dimensions n x 3 with n being number of points
% 
% Written in 2019 by Matthias Schaufelberger
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function [face_indices_out] = vtkLineOfBlock(mesh,point_array,radius) %#ok
