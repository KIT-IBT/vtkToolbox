% Transforms a dataset the same way as the "Transform" filter in Paraview:
%   1) scaling
%   2) rotation around y
%   3) rotation around x
%   4) rotation around z
%   5) translation
%
% Syntax:
% outStruct = vtkTransform(struct inStruct, double t, double r, double s)
%
% t: 1x3 vector of translations in x,y,z
% r: 1x3 vector of rotation angles around x,y,z in degrees
% s: 1x3 vector of scale factors in x,y,z
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function [outStruct,R] = vtkTransform(inStruct, t, r, s)

R = makehgtform('translate', t) * ...
    makehgtform('zrotate', r(3)/180*pi) * ...
    makehgtform('xrotate', r(1)/180*pi) * ...
    makehgtform('yrotate', r(2)/180*pi) * ...
    makehgtform('scale', s);

P = inStruct.points';
P = R * [P; ones(1,size(P,2))];
P = P(1:3,:)';

outStruct = inStruct;
outStruct.points = P;

end