% Creates an icosahedron sphere using the S2-Sampling-Toolbox
%
% Syntax:
% outStruct = vtkCreateIcosphere(numSubdiv, radius, center)
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkCreateIcosphere(numSubdiv, radius, center)

    mpath = fileparts(mfilename('fullpath'));
    addpath([mpath '/S2-Sampling-Toolbox']);
    
    TR = SubdivideSphericalMesh(IcosahedronMesh, numSubdiv);
    outStruct = triangulationToVtk(TR);
    
    if nargin > 1
        outStruct.points = outStruct.points * radius;
    end
    if nargin > 2
        outStruct.points = outStruct.points + repmat(center(:)', size(outStruct.points,1), 1);
    end

end