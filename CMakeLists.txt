PROJECT (PerceptualDiff)
SET(DIFF_SRC PerceptualDiff.cpp LPyramid.cpp RGBAImage.cpp
CompareArgs.cpp Metric.cpp)

ADD_EXECUTABLE (perceptualdiff ${DIFF_SRC})

INSTALL(FILES perceptualdiff DESTINATION bin)

# look for libtiff
FIND_PATH(TIFF_INCLUDE_DIR tiff.h
  /usr/local/include
  /usr/include
  /opt/local/include
)

FIND_LIBRARY(TIFF_LIBRARY tiff
  /usr/lib
  /usr/local/lib
  /opt/local/lib
)

IF(TIFF_INCLUDE_DIR)
  IF(TIFF_LIBRARY)
    SET( TIFF_FOUND "YES" )
    SET( TIFF_LIBRARIES ${TIFF_LIBRARY} )
  ENDIF(TIFF_LIBRARY)
ENDIF(TIFF_INCLUDE_DIR)

IF(TIFF_FOUND)
  INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(perceptualdiff ${TIFF_LIBRARY})
ENDIF(TIFF_FOUND)

# look for libpng
FIND_PATH(PNG_INCLUDE_DIR png.h
  /usr/local/include
  /usr/include
  /opt/local/include
)

FIND_LIBRARY(PNG_LIBRARY png
  /usr/lib
  /usr/local/lib
  /opt/local/lib
)

IF(PNG_INCLUDE_DIR)
  IF(PNG_LIBRARY)
    SET( PNG_FOUND "YES" )
    SET( PNG_LIBRARIES ${PNG_LIBRARY} )
  ENDIF(PNG_LIBRARY)
ENDIF(PNG_INCLUDE_DIR)

IF(PNG_FOUND)
  INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(perceptualdiff ${PNG_LIBRARY})
ENDIF(PNG_FOUND)

#
# Packing stuff
#
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An image comparison utility.")
set(CPACK_PACKAGE_VENDOR "pdiff dev team")
set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/README.txt)
set(CPACK_GENERATOR TGZ)
set(
CPACK_SOURCE_PACKAGE_FILE_NAME
"PerceptualDiff-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
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


