/*
PerceptualDiff - a program that compares two images using a perceptual metric
based on the paper :
A perceptual metric for production testing. Journal of graphics tools,
9(4):33-40, 2004, Hector Yee
Copyright (C) 2006-2011 Yangli Hector Yee
Copyright (C) 2011-2014 Steven Myint

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "LPyramid.h"
#include "RGBAImage.h"
#include "CompareArgs.h"
#include "Metric.h"

#include <iostream>
#include <string>


int main(int argc, char **argv)
{
    CompareArgs args;

    if (not args.Parse_Args(argc, argv))
    {
        std::cout << args.ErrorStr;
        return -1;
    }
    else
    {
        if (args.Verbose)
        {
            args.Print_Args();
        }
    }

    const auto passed = Yee_Compare(args);
    if (passed)
    {
        if (args.Verbose)
        {
            std::cout << "PASS: " << args.ErrorStr;
        }
    }
    else
    {
        std::cout << "FAIL: " << args.ErrorStr;
    }

    return passed ? 0 : 1;
}
