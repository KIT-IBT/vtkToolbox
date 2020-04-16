% Deletes all point, cell and field data arrays except those optionally
% given in arraysToKeep.
%
% Syntax:
% outStruct = vtkDeleteDataArrays(struct inStruct, cell arraysToKeep)
%
% arraysToKeep: cell array of chars with names of arrays that should not be
%               deleted, e.g. {'cellData.Material','pointData.Fiber'}
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkDeleteDataArrays(inStruct, arraysToKeep)

outStruct.points = inStruct.points;
outStruct.cells = inStruct.cells;
outStruct.cellTypes = inStruct.cellTypes;

if nargin > 1
    for i = 1:numel(arraysToKeep)
        eval(sprintf('outStruct.%s = inStruct.%s;', arraysToKeep{i}, arraysToKeep{i}));
    end
end

end