#!/bin/bash -ex
#
# Run tests for Coveralls.

./coverage.bash

path=$PWD
result_path="$path/CMakeFiles/perceptualdiff.dir"

for f in *.cpp *.h
do
    base=$(basename "$f" .cpp)
    base=$(basename "$base" .h)
    cd "$result_path"
    gcov "$base"
done

# Do this in a separate pass to make sure everything is accumulated.
cd "$path"
# Ignore the header files, which don't seem to have correct coverage results.
for f in *.cpp
do
    cp "$result_path/$f.gcov" "$path" || true
done

coveralls --verbose --no-gcov --exclude='CMakeFiles'
