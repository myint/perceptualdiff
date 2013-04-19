#!/bin/bash -ex
#
# Run tests with coverage reporting enabled.

export CXXFLAGS='-coverage'
export LDFLAGS='-coverage'

rm -f CMakeCache.txt
rm -rf CMakeFiles

cmake -D CMAKE_BUILD_TYPE=Debug .

make check

# Test output.
./perceptualdiff -output diff.png -verbose test/fish[12].png
./perceptualdiff -verbose -scale -sum-errors test/fish1.png test/Aqsis_vase.png

# For Coveralls.
path=$PWD
for f in *.cpp *.h
do
    base=$(basename "$f" .cpp)
    base=$(basename "$base" .h)
    gcov "$base"
    cd "$path/CMakeFiles/perceptualdiff.dir"
    cp "$f.gcov" "$path"
done
