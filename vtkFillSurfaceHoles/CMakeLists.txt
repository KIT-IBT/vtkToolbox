FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})


#FIND_PACKAGE(Boost COMPONENTS serialization REQUIRED)
find_package(Eigen3 REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR})
#include_directories(${Boost_INCLUDE_DIRS})

set(SOURCES
HoleFillerDefines.h
UmbrellaWeightedOrder2Smoother.h
UmbrellaWeightedOrder2Smoother.cpp
SurfaceHoleFiller.h  
SurfaceHoleFiller.cpp 
FillSurfaceHoles.cpp
CoverRefiner.h
CoverRefiner.cpp
)

ADD_EXECUTABLE(FillSurfaceHoles ${SOURCES})
TARGET_LINK_LIBRARIES(FillSurfaceHoles ${VTK_LIBRARIES})
#TARGET_LINK_LIBRARIES(FillSurfaceHoles ${VTK_LIBRARIES} ${Boost_LIBRARIES})

