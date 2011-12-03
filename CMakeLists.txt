PROJECT (PerceptualDiff)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

set(DIFF_SRC PerceptualDiff.cpp LPyramid.cpp RGBAImage.cpp
CompareArgs.cpp Metric.cpp)

add_executable (perceptualdiff ${DIFF_SRC})

install(TARGETS perceptualdiff DESTINATION bin)

# look for freeimage
find_path(FREEIMAGE_INCLUDE_DIR FreeImage.h
  /usr/local/include
  /usr/include
  /opt/local/include
)
find_library(FREEIMAGE_LIBRARY freeimage
  /usr/lib
  /usr/local/lib
  /opt/local/lib
)

if(FREEIMAGE_INCLUDE_DIR)
  if(FREEIMAGE_LIBRARY)
    set(FREEIMAGE_FOUND "YES")
    set(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBRARY})
  endif(FREEIMAGE_LIBRARY)
endif(FREEIMAGE_INCLUDE_DIR)

if(FREEIMAGE_FOUND)
  include_directories(${FREEIMAGE_INCLUDE_DIR})
  target_link_libraries(perceptualdiff ${FREEIMAGE_LIBRARY})
endif(FREEIMAGE_FOUND)

option(DIFF_USE_OPENMP "Enable parallel processing via OpenMP." TRUE)

if(DIFF_USE_OPENMP)
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
  endif()
endif()

# Compiler warnings
if(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -ansi -pedantic -Wcast-qual -Wpointer-arith -Winit-self -Wswitch-default -Wmissing-include-dirs -Wold-style-cast -Wnon-virtual-dtor")
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
"/\\\\.gitignore$"
"/\\\\..*\\\\.swp$"
"^${PROJECT_SOURCE_DIR}/\\\\.git/"
"^${PROJECT_SOURCE_DIR}/CMakeFiles/"
"^${PROJECT_SOURCE_DIR}/CMakeCache.txt"
"^${PROJECT_SOURCE_DIR}/CPackSourceConfig.cmake"
"^${PROJECT_SOURCE_DIR}/CPackConfig.cmake"
"^${PROJECT_SOURCE_DIR}/CTestTestfile.cmake"
"^${PROJECT_SOURCE_DIR}/Makefile"
"^${PROJECT_SOURCE_DIR}/_CPack_Packages"
"^${PROJECT_SOURCE_DIR}/test/"
"^${PROJECT_SOURCE_DIR}/cppcheck.bash"
"^${PROJECT_SOURCE_DIR}/cmake_install.cmake"
"^${PROJECT_SOURCE_DIR}/install_manifest.txt"
"^${PROJECT_SOURCE_DIR}/perceptualdiff"
"^${PROJECT_SOURCE_DIR}/PerceptualDiff-.*"
"^${PROJECT_SOURCE_DIR}/Testing"
)
include(CPack)

# Tests
enable_testing()
add_test(run_tests.bash, "test/run_tests.bash")
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
                  DEPENDS "perceptualdiff")
