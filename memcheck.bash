#!/bin/bash -ex
#
# Run Valgrind memcheck on perceptualdiff.

VALGRIND="valgrind --error-exitcode=2 --quiet --leak-check=full \
    --track-origins=yes --gen-suppressions=all --suppressions=memcheck.supp"

$VALGRIND ./perceptualdiff test/cam_mb.tif test/cam_mb_ref.tif

$VALGRIND ./perceptualdiff --verbose --downsample 9 \
    test/fish1.png test/fish2.png
