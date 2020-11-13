function vtk = remeshTriangleMesh(vtk, numPoints, extraFlags)

if nargin < 3 || isempty(extraFlags)
    extraFlags = '';
end
if ~isempty(numPoints)
    extraFlags = sprintf('%s -v %i', extraFlags, round(numPoints));
end

% make sure the source mesh contains at least 4 times as many triangles as
% the target mesh (needed for good results with Instant Meshes)
numPointsSource = size(vtk.cells,1);
numSubdiv = max(1-floor(log(numPointsSource/numPoints)/log(4)),0);
if numSubdiv
    try
        vtk = vtkLinearSubdivisionFilter(vtk, numSubdiv);
    catch
        warning('Skipping subdivision before remeshing.');
    end
end

tmpfile = [tempname '.ply'];
vtkWrite(vtk, tmpfile);

instantmeshes = 'instantmeshes';
[noAliasFound,~] = system(sprintf('which %s', instantmeshes));
if noAliasFound
    instantmeshes = '/Applications/Instant Meshes.app/Contents/MacOS/Instant Meshes';
end
if ~exist(instantmeshes, 'file')
    instantmeshes = '/Users/ss029/Applications/Instant Meshes.app/Contents/MacOS/Instant Meshes';
%     instantmeshes = '/Volumes/ServerApps/Instant Meshes.app/Contents/MacOS/Instant Meshes';
end
if ~exist(instantmeshes, 'file')
    error('Instant Meshes could not be found.');
end

system(sprintf('''%s'' %s -o %s -r 6 -p 6 -S 0 -d %s', instantmeshes, tmpfile, tmpfile, extraFlags));

vtk = vtkRead(tmpfile);
delete(tmpfile);

end
