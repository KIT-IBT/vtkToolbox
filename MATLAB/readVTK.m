% Reads a VTK file (legacy: vtk or XML: vtp, vtu).
% Also supported are: ply, stl, obj, (off not yet)
%
% Syntax:
% outStruct = readVTK(char filename, (bool verbose))
%
% Defaults:
% verbose = false
%
% Written in 2017 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = readVTK(filename, verbose)

if nargin < 2
    verbose = false;
end
    
outStruct = vtkRead(filename, verbose);

end
