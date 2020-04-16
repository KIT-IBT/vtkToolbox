function edgeLengths = vtkEdgeLengths(inStruct)

TR = vtkToTriangulation(inStruct);
edges = TR.edges;
edgeLengths = sqrt(sum((TR.Points(edges(:,1),:)-TR.Points(edges(:,2),:)).^2,2));

end