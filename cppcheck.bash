#!/bin/bash
#
# Run cppcheck on the source code.

cppcheck --enable=style,performance,portability --std=posix ./*.cpp
