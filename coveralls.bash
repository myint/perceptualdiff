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
for f in *.cpp *.h
do
    cp "$result_path/$f.gcov" "$path" || true
done
