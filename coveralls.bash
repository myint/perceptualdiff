#!/bin/bash -ex
#
# Run tests with coverage reporting enabled.

export CXXFLAGS='-coverage'
export LDFLAGS='-coverage'

rm -f CMakeCache.txt
rm -rf CMakeFiles

cmake -D CMAKE_BUILD_TYPE=Debug .

make check

# For Coveralls.
path=$PWD
for f in *.cpp *.h
do
    base=$(basename "$f" .cpp)
    base=$(basename "$base" .h)
    gcov "$base"
    cd "$path/CMakeFiles/perceptualdiff.dir"
    cp "$f.gcov" "$path" || true
done
