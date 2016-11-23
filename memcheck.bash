#!/bin/bash
#
# Run Valgrind memcheck on perceptualdiff.

set -eux

VALGRIND="valgrind --error-exitcode=2 --quiet --leak-check=full \
    --track-origins=yes --gen-suppressions=all --suppressions=memcheck.supp"

for d in ./build .
do
    pdiff="$d/perceptualdiff"
    if [ -f "$pdiff" ]; then
        break
    fi
done

$VALGRIND "$pdiff" test/cam_mb.tif test/cam_mb_ref.tif

$VALGRIND "$pdiff" --verbose --downsample 9 test/fish1.png test/fish2.png
