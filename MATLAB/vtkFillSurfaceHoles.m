% Detects and fills holes in a vtkPolyData.
% You can choose, whether/how to smooth the filled-in surface.
% 
% This function uses Constantine Butakoff's code from
% https://github.com/cbutakoff/tools/tree/master/FillSurfaceHoles (commit 8c2cac1),
% which implements the algorithms in
% P. Liepa (2003), Filling Holes in Meshes, http://dx.doi.org/10.2312/SGP/SGP03/200-206
% G. Barequet et al. (1995), Filling gaps in the boundary of a polyhedron, https://doi.org/10.1016/0167-8396(94)00011-G
%
% Syntax:
% outStruct = vtkFillSurfaceHoles(struct inStruct, (char method), (bool verbose))
%
% method:  smoothing method:
%          'none':    no smoothing
%          'cot':     cotangent weights (default)
%          'edgelen': inverse edge length weights
% verbose: whether to print status messages (default: false)
%
% Written in 2021 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkFillSurfaceHoles(inStruct, method, verbose) %#ok
