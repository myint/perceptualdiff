#!/bin/bash -ex
#
# Run Valgrind memcheck on perceptualdiff.

valgrind --error-exitcode=2 --quiet --leak-check=full --gen-suppressions=all --suppressions=memcheck.supp ./perceptualdiff test/cam_mb.tif test/cam_mb_ref.tif
