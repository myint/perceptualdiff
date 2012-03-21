#!/bin/bash
#
# Run Valgrind helgrind on perceptualdiff.

valgrind --error-exitcode=123 --quiet --tool=helgrind --gen-suppressions=all ./perceptualdiff test/cam_mb.tif test/cam_mb_ref.tif
