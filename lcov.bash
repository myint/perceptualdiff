#!/bin/bash -ex
#
# Run tests with coverage reporting enabled.

./coverage.bash

# Create HTML report.
lcov --directory='CMakeFiles/perceptualdiff.dir' --output-file='lcov_tmp.info' --capture
lcov --output-file='lcov.info' --extract 'lcov_tmp.info' "$(pwd)/*"
genhtml --output-directory='coverage_output' lcov.info

# Remove intermediate lcov output.
rm 'lcov_tmp.info'

python -m webbrowser -n "file://${PWD}/coverage_output/index.html"
