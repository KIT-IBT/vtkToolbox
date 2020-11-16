% Integrates a vector field to generate streamlines.
% If the data array provided represents a scalar field, the stream tracer 
% automatically operates on its gradient field.
% The input is an unstructured grid containing a point or cell data array
% to operate on, a poly data containing seed points and the fixed step
% length used for integration using Runge-Kutta 4.
% The output is a poly data consisting of line segments.
%
% Syntax:
% outStruct = vtkStreamTracer(inStruct1, cellsOrPoints, arrayName,
%                             inStruct2, direction, stepLength,
%                             endPointPrecision, maxLength, maxNumSteps)
%
% inStruct1: unstructured grid
% cellsOrPoints: 'cells' or 'points'
% arrayName: name of the data array to compute streamlines on;
%            if it is a vector field, the stream tracer is applied directly;
%            if it is a scalar field, the stream tracer is applied to its gradient
% inStruct2: polydata with seed points
% direction: direction of integration: 'forward', 'backward' or 'both'
% stepLength: fixed step length used for streamline integration (in length unit)
% endPointPrecision: Step length for a secondary run of vtkStreamTracer,
%                    starting from the end points of the first run.
%                    Original end points will then be replaced by the end points
%                    of the secondary run. This can be used to reduce the gap 
%                    between the ends of streamlines and the domain boundary.
%                    (optional, default: -1, i.e. no secondary run)
% maxLength: maximum propagation length (optional, default: 1e3*stepLength)
% maxNumSteps: maximum number of steps for a streamline (optional, default: 2000)
%
% Written in 2020 by Steffen Schuler
% Institute of Biomedical Engineering, KIT
% www.ibt.kit.edu

function outStruct = vtkStreamTracer(inStruct1, cellsOrPoints, arrayName, inStruct2, direction, stepLength, endPointPrecision, maxLength, maxNumSteps) %#ok
