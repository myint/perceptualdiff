#!/bin/bash -eux

rm -rf perceptualdiff.tgz cov-int
cov-build --dir cov-int g++ -std=c++0x -o perceptualdiff ./*.cpp -lfreeimage
tar czvf perceptualdiff.tgz cov-int
