#!/bin/bash -eux

# Script to run pdiff against a set of image file pairs, and check that the
# PASS or FAIL status is as expected.

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
EOF
}

# Change to test directory
script_directory=$(dirname "$0")
cd "$script_directory"

if [ -f '../build/perceptualdiff' ]
then
	pdiff=../build/perceptualdiff
elif [ -f '../perceptualdiff' ]
then
	pdiff=../perceptualdiff
else
	echo 'perceptualdiff must be built and exist in the repository root or the "build" directory'
	exit 1
fi

#------------------------------------------------------------------------------

totalTests=0
numTestsFailed=0

# Run all tests.
while read expectedResult image1 image2 ; do
	if $pdiff --verbose --scale "$image1" "$image2" 2>&1 | grep -q "^$expectedResult" ; then
		totalTests=$((totalTests+1))
	else
		numTestsFailed=$((numTestsFailed+1))
		echo "Regression failure: expected $expectedResult for \"$pdiff $image1 $image2\"" >&2
	fi
done <<EOF
$(all_tests)
EOF
# (the above with the EOF's is a stupid bash trick to stop while from running
# in a subshell)

# Give some diagnostics:
if [[ $numTestsFailed == 0 ]] ; then
	echo "*** all $totalTests tests passed"
else
	echo "*** $numTestsFailed failed tests of $totalTests"
	exit $numTestsFailed
fi

# Run additional tests.
$pdiff 2>&1 | grep -i openmp
rm -f diff.png
$pdiff --output diff.png --verbose fish[12].png 2>&1 | grep -q 'FAIL'
ls diff.png
rm -f diff.png

head fish1.png > fake.png
$pdiff --verbose fish1.png fake.png 2>&1 | grep -q 'Failed to load'
rm -f fake.png

mkdir -p unwritable.png
$pdiff --output unwritable.png --verbose fish[12].png 2>&1 | grep -q 'Failed to save'
rmdir unwritable.png

$pdiff fish[12].png --output foo 2>&1 | grep -q 'unknown filetype'
$pdiff --verbose fish1.png 2>&1 | grep -q 'Not enough'
$pdiff --downsample -3 fish1.png Aqsis_vase.png 2>&1 | grep -q 'Invalid'
$pdiff --threshold -3 fish1.png Aqsis_vase.png 2>&1 | grep -q 'Invalid'
$pdiff cam_mb_ref.tif cam_mb.tif --fake-option
$pdiff --verbose --scale fish1.png Aqsis_vase.png 2>&1 | grep -q 'FAIL'
$pdiff --downsample 2 fish1.png Aqsis_vase.png 2>&1 | grep -q 'FAIL'
$pdiff  /dev/null /dev/null 2>&1 | grep -q 'FAIL'
$pdiff --verbose --sum-errors fish[12].png 2>&1 | grep -q 'sum'
$pdiff --colorfactor .5 -threshold 1000 --gamma 3 --luminance 90 cam_mb_ref.tif cam_mb.tif
$pdiff --verbose -downsample 30 -scale --luminanceonly --fov 80 cam_mb_ref.tif cam_mb.tif
$pdiff --fov wrong fish1.png fish1.png 2>&1 | grep -q 'Invalid argument'

echo -e '\x1b[01;32mOK\x1b[0m'
