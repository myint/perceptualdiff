PROJECT (PerceptualDiff)
SET(DIFF_SRC PerceptualDiff.cpp LPyramid.cpp RGBAImage.cpp
CompareArgs.cpp Metric.cpp)

ADD_EXECUTABLE (pdiff ${DIFF_SRC})

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
  TARGET_LINK_LIBRARIES(pdiff ${TIFF_LIBRARY})
ENDIF(TIFF_FOUND)