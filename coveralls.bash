#!/bin/bash -ex
#
# Run tests for Coveralls.

./coverage.bash

path=$PWD
for f in *.cpp *.h
do
    base=$(basename "$f" .cpp)
    base=$(basename "$base" .h)
    gcov "$base"
    cd "$path/CMakeFiles/perceptualdiff.dir"
    cp "$f.gcov" "$path" || true
done
