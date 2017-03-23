#!/bin/bash

set -eux

# Script to run pdiff against a set of image file pairs, and check that the
# PASS or FAIL status is as expected.

trap "echo -e '\x1b[01;31mFailed\x1b[0m'" ERR

#------------------------------------------------------------------------------
# Image files and expected perceptualdiff PASS/FAIL status.  Line format is
# (PASS|FAIL) image1.(tif|png) image2.(tif|png)
#
# Edit the following lines to add additional tests.
all_tests () {
cat <<EOF
FAIL Bug1102605_ref.tif Bug1102605.tif
PASS Bug1471457_ref.tif Bug1471457.tif
PASS cam_mb_ref.tif cam_mb.tif
FAIL fish2.png fish1.png
PASS square.png square_scaled.png
FAIL Aqsis_vase.png Aqsis_vase_ref.png
FAIL alpha1.png alpha2.png
EOF
}

# Change to test directory
readonly script_directory=$(dirname "$0")
cd "$script_directory"

found=0
for d in ../build .. ../obj*; do
    pdiff="$d/perceptualdiff"
    if [ -f "$pdiff" ]; then
	found=1
	break
    fi
done

if [ $found = 0 ]; then
    echo 'perceptualdiff must be built and exist in the repository root or the "build" directory'
    exit 1
else
    echo "*** testing executable binary $pdiff"
fi

#------------------------------------------------------------------------------

total_tests=0
num_tests_failed=0

# Run all tests.
while read expected_result image1 image2 ; do
	if "$pdiff" --verbose --scale "$image1" "$image2" 2>&1 | grep -q "^$expected_result" ; then
		total_tests=$((total_tests+1))
	else
		num_tests_failed=$((num_tests_failed+1))
		echo "Regression failure: expected $expected_result for \"$pdiff $image1 $image2\"" >&2
	fi
done <<EOF
$(all_tests)
EOF
# (the above with the EOF's is a stupid bash trick to stop while from running
# in a subshell)

# Give some diagnostics:
if [[ $num_tests_failed == 0 ]] ; then
	echo "*** all $total_tests tests passed"
else
	echo "*** $num_tests_failed failed tests of $total_tests"
	exit $num_tests_failed
fi

# Run additional tests.
"$pdiff" 2>&1 | grep -i 'openmp'
"$pdiff" --version | grep -i 'perceptualdiff'
"$pdiff" --help | grep -i 'usage'

rm -f diff.png
"$pdiff" --output diff.png --verbose fish[12].png 2>&1 | grep -q 'FAIL'
ls diff.png
rm -f diff.png

head fish1.png > fake.png
"$pdiff" --verbose fish1.png fake.png 2>&1 | grep -q 'Failed to load'
rm -f fake.png

mkdir -p unwritable.png
"$pdiff" --output unwritable.png --verbose fish[12].png 2>&1 | grep -q 'Failed to save'
rmdir unwritable.png

"$pdiff" fish[12].png --output foo 2>&1 | grep -q 'unknown filetype'
"$pdiff" --verbose fish1.png 2>&1 | grep -q 'Not enough'
"$pdiff" --down-sample -3 fish1.png Aqsis_vase.png 2>&1 | grep -q 'Invalid'
"$pdiff" --threshold -3 fish1.png Aqsis_vase.png 2>&1 | grep -q 'Invalid'
"$pdiff" cam_mb_ref.tif cam_mb.tif --fake-option
"$pdiff" --verbose --scale fish1.png Aqsis_vase.png 2>&1 | grep -q 'FAIL'
"$pdiff" --down-sample 2 fish1.png Aqsis_vase.png 2>&1 | grep -q 'FAIL'
"$pdiff"  /dev/null /dev/null 2>&1 | grep -q 'Unknown filetype'
"$pdiff" --verbose --sum-errors fish[12].png 2>&1 | grep -q 'sum'
"$pdiff" --color-factor .5 -threshold 1000 --gamma 3 --luminance 90 cam_mb_ref.tif cam_mb.tif
"$pdiff" --verbose -down-sample 30 -scale --luminance-only --fov 80 cam_mb_ref.tif cam_mb.tif
"$pdiff" --fov wrong fish1.png fish1.png 2>&1 | grep -q 'Invalid argument'

echo -e '\x1b[01;32mOK\x1b[0m'
