#!/bin/bash -ex

# clang-format doesn't handle default arguments properly. So don't touch the
# headers files.
clang-format -i *.cpp
