function [center,radius] = vtkSmallestEnclosingSphere(inStruct)

points = double(inStruct.points);
objFun = @(center) max(sqrt(sum((points-center).^2,2)));
options = optimset('MaxFunEvals', 1e4);
[center,radius] = fminsearch(objFun, mean(points,1), options);

end