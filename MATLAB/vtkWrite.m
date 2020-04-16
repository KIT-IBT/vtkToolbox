% Writes a VTK file (legacy: vtk or XML: vtp, vtu).
% Also supported are: ply, stl, obj, off (only geometry, no data arrays)
%
% Syntax:
% vtkWrite(struct inStruct, char filename, (bool verbose), (char dataMode));
%
% inStruct must have the following fields:
%     points:    [numPoints x numDim double/single/(u)intN], numDim <= 3
%     cells:     [numCells x maxNumPointsPerCell int32]
%     cellTypes: [numCells x 1 uint8] (see https://vtk.org/doc/nightly/html/vtkCellType_8h_source.html)
%     pointData: [1 x 1 struct of point data arrays] (optional)
%     cellData:  [1 x 1 struct of cell data arrays] (optional)
%     fieldData: [1 x 1 struct of field data arrays] (optional)
%
% Defaults:
% verbose = false
% 
% dataMode can be chosen from {'ascii', 'binary'} for vtk, ply, stl
%                         and {'ascii', 'binary', 'appended'} for vtp, vtu
%                         and {'ascii'} for obj, off
%
% Written in 2017 by Steffen Schuler, Nils Daub
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function vtkWrite(inStruct, filename, verbose, dataMode) %#ok
