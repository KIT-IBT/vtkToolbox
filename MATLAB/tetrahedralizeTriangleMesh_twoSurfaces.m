function vtk_vol = tetrahedralizeTriangleMesh_twoSurfaces(vtk_inner_sur, vtk_outer_sur, relativeEdgeLength, optimizeThreshold, optimizeNetgen)

if nargin < 5
    optimizeNetgen = false;
end
if nargin < 4 || isempty(optimizeThreshold)
    optimizeThreshold = 1;
end
if nargin < 3 || isempty(relativeEdgeLength)
    relativeEdgeLength = 1;
end

[tmpdir,plyname] = fileparts(tempname);
plyfile_inner = [tmpdir '/' plyname '_inner.ply'];
vtkWrite(vtk_inner_sur, plyfile_inner, false, 'ascii');
plyfile_outer = [tmpdir '/' plyname '_outer.ply'];
vtkWrite(vtk_outer_sur, plyfile_outer, false, 'ascii');

[~,geoname] = fileparts(tempname);
geofile = [tmpdir '/' geoname '.geo'];
fid = fopen(geofile, 'w');
fprintf(fid, 'Merge "%s_inner.ply";\nMerge "%s_outer.ply";\nSurface Loop(1) = {1,2};\nVolume(1) = {1};', plyname, plyname);
fclose(fid);

meanEdgeLength = (mean(vtkEdgeLengths(vtk_inner_sur))+mean(vtkEdgeLengths(vtk_outer_sur)))/2;
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

delete(plyfile_inner);
delete(plyfile_outer);
delete(geofile);
delete(vtkfile);

end