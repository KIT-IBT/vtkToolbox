% Reads a VTK file (legacy: vtk or XML: vtp, vtu).
% Also supported are: ply, stl, obj, (off not yet)
%%
% Syntax:
% outStruct = readVTK(char filename, (bool verbose))
% filename may be a char vector or a string scalar.
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

    if isstring(filename)
        if ~isscalar(filename)
            error('vtkToolbox:vtkRead:InvalidFilenameType', ...
                'filename must be a char vector or a string scalar.');
        end
        filename = char(filename);
    end

    if ~(ischar(filename) || (isstring(filename) && isscalar(filename)))
        error('vtkToolbox:vtkRead:InvalidFilenameType', ...
            'filename must be a char vector or a string scalar.');
    end

    outStruct = feval(mfilename, filename, verbose);

end
