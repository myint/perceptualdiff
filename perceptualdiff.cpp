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

#include "compare_args.h"
#include "lpyramid.h"
#include "metric.h"
#include "rgba_image.h"

#include <cstdlib>
#include <iostream>
#include <string>


int main(int argc, char **argv)
{
    CompareArgs args;

    try
    {
        if (not args.parse_args(argc, argv))
        {
            std::cerr << args.error_string_;
            return EXIT_FAILURE;
        }
        else
        {
            if (args.verbose_)
            {
                args.print_args();
            }
        }

        const auto passed = yee_compare(args);
        if (passed)
        {
            if (args.verbose_)
            {
                std::cout << "PASS: " << args.error_string_;
            }
        }
        else
        {
            std::cout << "FAIL: " << args.error_string_;
        }

        return passed ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    catch (const ParseException &exception)
    {
        std::cerr << exception.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const RGBImageException &exception)
    {
        std::cerr << exception.what() << std::endl;
        return EXIT_FAILURE;
    }
}
