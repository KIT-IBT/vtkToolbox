% Wrapper of MATLAB's trisurf function, allowing a reasonable
% visualization of data on triangle meshes, optionally with isocontours.
% All name-value pairs of trisurf() are available.
% Addional parameters available:
% 'ViewAngles', 'Limits', 'Steps', 'ShowAxes',
% 'ShowContours', 'ContourColor', 'ContourWidth'
%
% Example:
% h = vtkTrisurf(vtk, 'CData',data, 'ShowContours',true);
%
% Written in 2021 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function h = vtkTrisurf(vtk, varargin)

p = inputParser;
addParameter(p, 'FaceColor', 'interp');
addParameter(p, 'FaceAlpha', 1);
addParameter(p, 'EdgeColor', 'none');
addParameter(p, 'SpecularStrength', 0.3);
addParameter(p, 'ViewAngles', []);
addParameter(p, 'Limits', []);
addParameter(p, 'Steps', 20);
addParameter(p, 'ShowAxes', false);
addParameter(p, 'ShowContours', false);
addParameter(p, 'ContourColor', [0 0 0]);
addParameter(p, 'ContourWidth', 0.7);

h = trisurf(vtk.cells, vtk.points(:,1), vtk.points(:,2), vtk.points(:,3));

for f = setdiff(fieldnames(h), p.Parameters)'
    addParameter(p, f{1}, []);
end

parse(p, varargin{:});
p = p.Results;

for f = intersect(fieldnames(h), fieldnames(p))'
    if ~isempty(p.(f{1}))
        h.(f{1}) = p.(f{1});
    end
end

lighting gouraud
light('Position',[1 -1/sqrt(3) -1/sqrt(6)]);
light('Position',[-1 -1/sqrt(3) -1/sqrt(6)]);
light('Position',[0 2/sqrt(3) -1/sqrt(6)]);
light('Position',[0 0 3/sqrt(6)]);
camproj('persp');
axis('equal');
if p.ShowAxes
    axis('on');
else
    axis('off');
end

if ~isempty(p.ViewAngles)
    view(p.ViewAngles(1), p.ViewAngles(2));
end

if isempty(p.Limits)
    p.Limits = [min(h.CData(:)) max(h.CData(:))];
end
caxis(p.Limits);

colormap(flipud(turbo(p.Steps)));

if p.ShowContours && ~isempty(p.CData)
        if size(p.CData,1) == size(vtk.points,1)
            vtk.pointData.data = p.CData;
            c = vtkContourFilter(vtk, 'points', 'data', linspace(p.Limits(1), p.Limits(2), p.Steps+1));
        elseif size(p.CData,1) == size(vtk.cells,1)
            vtk.cellData.data = p.CData;
            c = vtkContourFilter(vtk, 'cells', 'data', linspace(p.Limits(1), p.Limits(2), p.Steps+1));
        end
    hold on;
    trisurf(c.cells, c.points(:,1), c.points(:,2), c.points(:,3), 'EdgeColor',p.ContourColor, 'LineWidth',p.ContourWidth);
    hold off;
end

end