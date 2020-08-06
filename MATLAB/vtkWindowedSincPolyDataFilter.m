% Adjusts point coords using a windowed sinc function interpolation kernel.
% The effect is to "relax" the mesh, making the cells better shaped and the 
% vertices more evenly distributed.
%
% Syntax:
% outStruct = vtkWindowedSincPolyDataFilter(struct inStruct,
%                                           (int numberOfIterations)),
%                                           (double passband),
%                                           (bool featureEdgeSmoothing),
%                                           (double featureAngle),
%                                           (double edgeAngle),
%                                           (bool boundarySmoothing))
%
% Important inputs:
% numberOfIterations: Max. number of smoothing passes. Corresponds to the 
%                     polynomial degree used to approximate the windowed sinc.
%                     10 to 20 iterations are usually sufficient.
% passband:           Defines the pass band of the sinc filter.
%                     Must be between 0 and 2. A reasonable value is 0.1.
%
% Defaults:
% numberOfIterations = 20
% passband = 0.1
% featureEdgeSmoothing = false
% featureAngle = 45 (degrees)
% edgeAngle = 15 (degress)
% boundarySmoothing = true
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkWindowedSincPolyDataFilter(inStruct, numberOfIterations, passband, featureEdgeSmoothing, featureAngle, edgeAngle, boundarySmoothing) %#ok
