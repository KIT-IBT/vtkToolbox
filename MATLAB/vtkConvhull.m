% Computes the convex hull of a point set using MATLAB's convhull function.
% The result is a triangulation (vtkPolyData).
%
% Syntax:
% outStruct = vtkConvhull(struct inStruct)
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkConvhull(inStruct)

c = convhull(double(inStruct.points));
oldInd = unique(c(:));
newInd = 1:numel(oldInd);

outStruct.points = inStruct.points(oldInd,:);
outStruct.cells = int32(changem(c, newInd, oldInd));
outStruct.cellTypes = repmat(uint8(5), size(outStruct.cells,1), 1);

end