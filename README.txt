PerceptualDiff - a program that compares two images using a perceptual metric based on the paper :
A perceptual metric for production testing. Journal of graphics tools, 9(4):33-40, 2004, Hector Yee
Copyright (C) 2006 Yangli Hector Yee

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program;
if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

Usage

pdiff image1.tif image2.tif [options]
-verbose : Turns on verbose mode
-fov deg: field of view, deg, in degrees. Usually between 10.0 to 85.0. This controls how much of the screen the oberserver is seeing. Front row of a theatre has a field of view of around 25 degrees. Back row has a field of view of around 60 degrees.
-threshold p : Sets the number of pixels, p, to reject. For example if p is 100, then the test fails if 100 or more pixels are perceptably different.
-gamma g : The gamma to use to convert to RGB linear space. Default is 2.2
-luminance l: The luminance of the display the observer is seeing. Default is 100 candela per meter squared
