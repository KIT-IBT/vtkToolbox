function vtk_vol = tetrahedralizeTriangleMesh(vtk_sur, relativeEdgeLength, optimizeThreshold, optimizeNetgen)

if nargin < 4
    optimizeNetgen = false;
end
if nargin < 3 || isempty(optimizeThreshold)
    optimizeThreshold = 0.3;
end
if nargin < 2 || isempty(relativeEdgeLength)
    relativeEdgeLength = 1;
end

[tmpdir,plyname] = fileparts(tempname);
plyfile = [tmpdir '/' plyname '.ply'];
vtkWrite(vtk_sur, plyfile, false, 'ascii');

[~,geoname] = fileparts(tempname);
geofile = [tmpdir '/' geoname '.geo'];
fid = fopen(geofile, 'w');
fprintf(fid, 'Merge "%s.ply";\nSurface Loop(1) = {1};\nVolume(1) = {1};', plyname);
fclose(fid);

meanEdgeLength = mean(vtkEdgeLengths(vtk_sur));
clmax = relativeEdgeLength*0.7*meanEdgeLength;

gmsh = 'gmsh';
[noAliasFound,~] = system(sprintf('which %s', gmsh));
if noAliasFound
    gmsh = '/Applications/Gmsh.app/Contents/MacOS/gmsh';
end
if ~exist(gmsh, 'file')
    gmsh = '/Volumes/ServerApps/Gmsh.app/Contents/MacOS/gmsh';
end
if ~exist(gmsh, 'file')
    error('Gmsh could not be found.');
end

if optimizeNetgen
    system(sprintf('%s %s -3 -order 1 -format vtk -v 4 -clmax %.3f -optimize_netgen -optimize_threshold %.3f', gmsh, geofile, clmax, optimizeThreshold));
else
    system(sprintf('%s %s -3 -order 1 -format vtk -v 4 -clmax %.3f -optimize_threshold %.3f', gmsh, geofile, clmax, optimizeThreshold));
end

vtkfile = [tmpdir '/' geoname '.vtk'];
vtk_vol = vtkRead(vtkfile);
vtk_vol.cells(vtk_vol.cellTypes~=10,:) = [];
vtk_vol.cellTypes(vtk_vol.cellTypes~=10,:) = [];

delete(plyfile);
delete(geofile);
delete(vtkfile);

end