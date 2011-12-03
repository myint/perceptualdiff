PROJECT (PerceptualDiff)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

SET(DIFF_SRC PerceptualDiff.cpp LPyramid.cpp RGBAImage.cpp
CompareArgs.cpp Metric.cpp)

ADD_EXECUTABLE (perceptualdiff ${DIFF_SRC})

INSTALL(TARGETS perceptualdiff DESTINATION bin)

# look for freeimage
FIND_PATH(FREEIMAGE_INCLUDE_DIR FreeImage.h
  /usr/local/include
  /usr/include
  /opt/local/include
)
FIND_LIBRARY(FREEIMAGE_LIBRARY freeimage
  /usr/lib
  /usr/local/lib
  /opt/local/lib
)

IF(FREEIMAGE_INCLUDE_DIR)
  IF(FREEIMAGE_LIBRARY)
    SET( FREEIMAGE_FOUND "YES" )
    SET( FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBRARY} )
  ENDIF(FREEIMAGE_LIBRARY)
ENDIF(FREEIMAGE_INCLUDE_DIR)

IF(FREEIMAGE_FOUND)
  INCLUDE_DIRECTORIES(${FREEIMAGE_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(perceptualdiff ${FREEIMAGE_LIBRARY})
ENDIF(FREEIMAGE_FOUND)

option(DIFF_USE_OPENMP "Enable parallel processing via OpenMP." TRUE)

if(DIFF_USE_OPENMP)
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -O3")
  endif()
endif()

# Compiler warnings
if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -ansi -pedantic -Wcast-qual -Wpointer-arith -Winit-self -Wswitch-default -Wconversion -Wmissing-include-dirs -Wold-style-cast -Wnon-virtual-dtor")
endif()

#
# Packing stuff
#
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An image comparison utility.")
set(CPACK_PACKAGE_VENDOR "pdiff dev team")
set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/README.rst)
set(CPACK_GENERATOR TGZ)
set(
CPACK_SOURCE_PACKAGE_FILE_NAME
"PerceptualDiff-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}"
CACHE INTERNAL "tarball basename"
)
set(CPACK_SOURCE_GENERATOR TGZ)
# The following components are regex's to match anywhere (unless anchored)
# in absolute path + filename to find files or directories to be excluded
# from source tarball.
set(CPACK_SOURCE_IGNORE_FILES
"~$"
"\\\\.cvsignore$"
"^${PROJECT_SOURCE_DIR}/CMakeFiles/"
"^${PROJECT_SOURCE_DIR}/CMakeCache.txt"
"^${PROJECT_SOURCE_DIR}/CPackSourceConfig.cmake"
"^${PROJECT_SOURCE_DIR}/CPackConfig.cmake"
"^${PROJECT_SOURCE_DIR}/Makefile"
"^${PROJECT_SOURCE_DIR}/_CPack_Packages"
"^${PROJECT_SOURCE_DIR}/test/"
"^${PROJECT_SOURCE_DIR}/cmake_install.cmake"
)
include(CPack)


