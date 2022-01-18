function vtk_vol = tetrahedralizeTriangleMesh_tetgen(vtk_sur, maxVolumeFactor)

if nargin < 2 || isempty(maxVolumeFactor)
    maxVolumeFactor = 3;
end

[tmpdir,plyname] = fileparts(tempname);
plyfile = [tmpdir '/' plyname '.ply'];
vtkWrite(vtk_sur, plyfile, false, 'ascii');

tetgen = 'tetgen';
[noAliasFound,~] = system(sprintf('which %s', tetgen));
if noAliasFound
    tetgen = '/Applications/tetgen/tetgen';
end
if ~exist(tetgen, 'file')
    error('Tetgen could not be found.');
end

maxVolume = maxVolumeFactor * mean(vtkEdgeLengths(vtk_sur))^3/(6*sqrt(2));
system(sprintf('%s -Yk -pq1.1 -a%.4f %s', tetgen, maxVolume, plyfile));

vtkfile = [tmpdir '/' plyname '.1.vtk'];
vtk_vol = vtkRead(vtkfile);
vtk_vol.cells(vtk_vol.cellTypes~=10,:) = [];
vtk_vol.cellTypes(vtk_vol.cellTypes~=10,:) = [];

delete(plyfile);
delete(vtkfile);

end