cmake_minimum_required(VERSION 3.3)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 11)
project(vtkToolbox)

# Move MEX binaries to vtkToolbox/MATLAB
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/MATLAB)

## Set up Matlab dependency
find_package(Matlab REQUIRED)
if(NOT Matlab_FOUND)
    message(FATAL_ERROR "Matlab could not be found. Try setting Matlab_ROOT_DIR.")
endif()

## Set up VTK dependency
# Select only VTK components (libraries to be linked to) that are needed.
# (Removing the COMPONENTS argument from find_package links all of them.)
# See folder structure in https://vtk.org/doc/nightly/html/files.html
# to find out to which component a VTK class belongs.
set(VTK_COMPONENTS
    vtkCommonCore
    vtkCommonDataModel
    vtkCommonExecutionModel
    vtkCommonMath
    vtkCommonMisc
    vtkCommonSystem
    vtkFiltersCore
    vtkFiltersExtraction
    vtkFiltersFlowPaths
    vtkFiltersGeneral
    vtkFiltersGeometry
    vtkFiltersModeling
    vtkIOCore
    vtkIOGeometry
    vtkIOLegacy
    vtkIOPLY
    vtkIOXML
)
find_package(VTK REQUIRED COMPONENTS ${VTK_COMPONENTS} NO_MODULE)
if(NOT VTK_FOUND)
    message(FATAL_ERROR "VTK could not be found. Try setting VTK_DIR.")
else()
    message(STATUS "Found VTK: " ${VTK_DIR})
endif()
include(${VTK_USE_FILE})

## Build library "vtkMatlab", which is used by all MEX functions in this toolbox
include_directories(libvtkMatlab ${Matlab_INCLUDE_DIRS})
add_library(vtkMatlab STATIC libvtkMatlab/common.cxx libvtkMatlab/vtkToStruct.cxx libvtkMatlab/structToVtk.cxx)
target_link_libraries(vtkMatlab ${VTK_LIBRARIES})

## Build MEX functions
matlab_add_mex(NAME vtkRead SRC vtkRead/vtkRead.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkWrite SRC vtkWrite/vtkWrite.cxx vtkWrite/additionalWriters/vtkOBJWriter.cxx vtkWrite/additionalWriters/vtkOFFWriter.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkAlignNodesWithElectrodes SRC vtkAlignNodesWithElectrodes/vtkAlignNodesWithElectrodes.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkAppendFilter SRC vtkAppendFilter/vtkAppendFilter.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkAppendPolyData SRC vtkAppendPolyData/vtkAppendPolyData.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkArrayMapperNearestNeighbor SRC vtkArrayMapperNearestNeighbor/vtkArrayMapperNearestNeighbor.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkBooleanOperationPolyDataFilter SRC vtkBooleanOperationPolyDataFilter/vtkBooleanOperationPolyDataFilter.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkCellCentroids SRC vtkCellCentroids/vtkCellCentroids.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkCellDataToPointData SRC vtkCellDataToPointData/vtkCellDataToPointData.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkCleanPolyData SRC vtkCleanPolyData/vtkCleanPolyData.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkCleanUnstructuredGrid SRC vtkCleanUnstructuredGrid/vtkCleanUnstructuredGrid.cxx vtkCleanUnstructuredGrid/vtkCleanUnstructuredGridClass.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkConnectivityFilter SRC vtkConnectivityFilter/vtkConnectivityFilter.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkContourFilter SRC vtkContourFilter/vtkContourFilter.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkDataSetSurfaceFilter SRC vtkDataSetSurfaceFilter/vtkDataSetSurfaceFilter.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkExtractEdges SRC vtkExtractEdges/vtkExtractEdges.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkFeatureEdges SRC vtkFeatureEdges/vtkFeatureEdges.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkStreamTracer SRC vtkStreamTracer/vtkStreamTracer.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkLinearSubdivisionFilter SRC vtkLinearSubdivisionFilter/vtkLinearSubdivisionFilter.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkPointDataToCellData SRC vtkPointDataToCellData/vtkPointDataToCellData.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkSmoothPolyDataFilter SRC vtkSmoothPolyDataFilter/vtkSmoothPolyDataFilter.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkThreshold SRC vtkThreshold/vtkThreshold.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
matlab_add_mex(NAME vtkTriangleFilter SRC vtkTriangleFilter/vtkTriangleFilter.cxx LINK_TO vtkMatlab ${VTK_LIBRARIES})
