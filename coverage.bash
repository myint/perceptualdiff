#!/bin/bash
#
# Run tests with coverage reporting enabled.

export CXXFLAGS='-coverage'
export LDFLAGS='-coverage'

rm -f CMakeCache.txt
rm -rf CMakeFiles

cmake -D CMAKE_BUILD_TYPE=Debug .

make check

# Create HTML report
lcov --directory='CMakeFiles/perceptualdiff.dir' --output-file='lcov_tmp.info' --capture
lcov --output-file='lcov.info' --extract 'lcov_tmp.info' "$(pwd)/*"
genhtml --output-directory='coverage_output' lcov.info

# Remove intermediate lcov output
rm 'lcov_tmp.info'

echo -e '\nCoverage report: coverage_output/index.html'
