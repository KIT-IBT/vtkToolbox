% Generates data arrays 'Ids' from point and cell ids.
% Ids start from 1 (Matlab convention, not VTK/C++).
%
% Syntax:
% outStruct = vtkGenerateIds(struct inStruct)
%
% Written in 2019 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkGenerateIds(inStruct)

outStruct = inStruct;
outStruct.pointData.Ids = int32(1:size(outStruct.points,1))';
outStruct.cellData.Ids = int32(1:size(outStruct.cells,1))';

end
