% Computes the center of volume (center of mass assuming uniform density)
% of a tetrahedral mesh or a closed triangle mesh.
% Relies on gptoolbox by Alec Jacobson.
%
% Syntax:
% [centerOfVolume,volume,v] = vtkCenterOfVolume(inStruct)
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function [centerOfVolume,volume,v] = vtkCenterOfVolume(inStruct)

P = double(inStruct.points);
C = inStruct.cells;

dim = size(C,2);
if dim == 3
    P = [P; mean(P,1)];
    C = [C repmat(size(P,1), size(C,1), 1)];
end

a = P(C(:,1),:);
b = P(C(:,2),:);
c = P(C(:,3),:);
d = P(C(:,4),:);
v = dot(a-d, cross(b-d, c-d, 2), 2)./6;
volume = sum(v);

if dim == 3
    d0 = repmat([0 0 0], size(C,1), 1);
    v0 = dot(a-d0, cross(b-d0, c-d0, 2), 2)./6;
    volume0 = sum(v0);
    if abs(volume-volume0) > 1e-5
        warning('The surface is not closed.');
    end
end

centroids = squeeze(mean(reshape(P(C,:),[],size(C,2),size(P,2)),2));

centerOfVolume = sum(repmat(v,1,3) .* centroids, 1) ./ volume;

end