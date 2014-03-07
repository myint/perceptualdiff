#!/bin/bash -eux

cov-build --dir cov-int g++ -std=c++0x -o perceptualdiff ./*.cpp -lfreeimage
tar czvf perceptualdiff.tgz cov-int
