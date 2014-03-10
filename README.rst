==============
perceptualdiff
==============

A program that compares two images using a perceptually based image metric.

.. image:: https://travis-ci.org/myint/perceptualdiff.png?branch=master
    :target: https://travis-ci.org/myint/perceptualdiff
    :alt: Build status

.. image:: https://coveralls.io/repos/myint/perceptualdiff/badge.png?branch=master
    :target: https://coveralls.io/r/myint/perceptualdiff
    :alt: Test coverage status

Copyright (C) 2006-2011 Yangli Hector Yee
Copyright (C) 2011-2014 Steven Myint

http://pdiff.sourceforge.net

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details in the
file gpl.txt.


Build Instructions
==================

#. Download cross platform make from http://www.cmake.org
#. Download freeimage from https://sourceforge.net/projects/freeimage
   (mac: ``brew install freeimage``,
   ubuntu: ``apt-get install libfreeimage-dev``)
#. Type ``cmake .``
#. Type ``make .``
#. To specify the install directory,
   use ``make install DESTDIR="/home/me/mydist"``


Usage
=====

Command line::

    perceptualdiff image1.(tif | png) image2.(tif | png) [options]
    --verbose : Turns on verbose mode
    --fov deg : Field of view, deg, in degrees. Usually between 10.0 to 85.0.
                This controls how much of the screen the observer is seeing.
                Front row of a theatre has a field of view of around 25
                degrees. Back row has a field of view of around 60 degrees.
    --threshold p : Sets the number of pixels, p, to reject. For example if p
                    is 100, then the test fails if 100 or more pixels are
                    perceptibly different.
    --gamma g : The gamma to use to convert to RGB linear space. Default is 2.2
    --luminance l : The luminance of the display the observer is seeing.
                    Default is 100 candela per meter squared
    --colorfactor : How much of color to use, 0.0 to 1.0, 0.0 = ignore color.
    --downsample : How many powers of two to down sample the image.
    --scale : Scale images to match each other's dimensions.
    --sum-errors : Print a sum of the luminance and color differences.
    --output foo.ppm : Saves the difference image to foo.ppm

Check that perceptualdiff is built with OpenMP support::

    $ ./perceptualdiff | grep -i openmp
    OpenMP status: enabled


Credits
=======

- Hector Yee, project administrator and originator - hectorgon.blogspot.com
- Scott Corley, for png file IO code
- Tobias Sauerwein, for make install, package_source Cmake configuration
- Cairo Team for bugfixes
- Jim Tilander, Rewrote the IO to use FreeImage.


Version History
===============

- 1.0 - Initial distribution
- 1.0.1 - Fixed off by one convolution error and libpng interface to 1.2.8
- 1.0.2 - [jt] Converted the loading and saving routines to use FreeImage
- 1.1 - Added colorfactor and downsample options. Also always output
  difference file if requested. Always print out differing pixels even if the
  test passes.
- 1.1.1 - Turn off color test in low lighting conditions.
- 1.1.2 - Add OpenMP parallel processing support and fix bugs.
- 1.2 - Add ``--sum-errors``, use more standard option style, and fix bugs.
