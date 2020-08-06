function vtk = remeshTriangleMesh(vtk, numPoints, creaseAngle)

if nargin < 3
    creaseAngle = [];
end

% make sure the source mesh contains at least 4 times as many triangles as
% the target mesh (needed for good results with Instant Meshes)
numPointsSource = size(vtk.cells,1);
numSubdiv = max(1-floor(log(numPointsSource/numPoints)/log(4)),0);
if numSubdiv
    vtk = vtkLinearSubdivisionFilter(vtk, numSubdiv);
end

tmpfile = [tempname '.ply'];
vtkWrite(vtk, tmpfile);

instantmeshes = 'instantmeshes';
[noAliasFound,~] = system(sprintf('which %s', instantmeshes));
if noAliasFound
    instantmeshes = '/Applications/Instant Meshes.app/Contents/MacOS/Instant Meshes';
end
if ~exist(instantmeshes, 'file')
    instantmeshes = '/Volumes/ServerApps/Instant Meshes.app/Contents/MacOS/Instant Meshes';
end
if ~exist(instantmeshes, 'file')
    error('Instant Meshes could not be found.');
end

if isempty(creaseAngle)
    system(sprintf('''%s'' %s -v %i -o %s -r 6 -p 6 -S 0 -d', instantmeshes, tmpfile, round(numPoints), tmpfile));
else
    system(sprintf('''%s'' %s -v %i -o %s -r 6 -p 6 -S 0 -d -c %.1f', instantmeshes, tmpfile, round(numPoints), tmpfile, creaseAngle));
end

vtk = vtkRead(tmpfile);
delete(tmpfile);

end
