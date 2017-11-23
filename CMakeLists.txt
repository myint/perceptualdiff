project(perceptualdiff)

cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT MSVC)
    add_compile_options(
        -Wall -Wextra -pedantic -Wcast-qual -Wpointer-arith -Winit-self
        -Wswitch-default -Wmissing-include-dirs -Wold-style-cast
        -Wnon-virtual-dtor -Wshadow -Wno-unknown-pragmas)
endif()

find_package(OpenMP)
if(OPENMP_FOUND)
    add_compile_options(${OpenMP_CXX_FLAGS})
    if(NOT MSVC)
        set(CMAKE_EXE_LINKER_FLAGS
            "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
    endif()
endif()

option(SANITIZERS "Enable sanitizers" FALSE)
if(SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(FREEIMAGE_FIND_REQUIRED, TRUE)
find_package(FreeImage)

add_library(pdiff lpyramid.cpp rgba_image.cpp metric.cpp)
target_include_directories(pdiff SYSTEM PRIVATE ${FREEIMAGE_INCLUDE_DIRS})
target_link_libraries(pdiff PRIVATE ${FREEIMAGE_LIBRARIES})

add_executable(perceptualdiff compare_args.cpp perceptualdiff.cpp)
target_link_libraries(perceptualdiff PRIVATE pdiff)

install(TARGETS perceptualdiff DESTINATION bin)

# Packing stuff.
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "2")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An image comparison utility.")
set(CPACK_PACKAGE_VENDOR "pdiff dev team")
set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/README.rst)
set(CPACK_GENERATOR TGZ)
set(CPACK_SOURCE_PACKAGE_FILE_NAME
    "perceptualdiff-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}"
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
    "^${PROJECT_SOURCE_DIR}/cmake_install.cmake"
    "^${PROJECT_SOURCE_DIR}/install_manifest.txt"
    "^${PROJECT_SOURCE_DIR}/perceptualdiff"
    "^${PROJECT_SOURCE_DIR}/perceptualdiff-.*"
    "^${PROJECT_SOURCE_DIR}/Testing"
    "^${PROJECT_SOURCE_DIR}/coverage_output"
)
include(CPack)
