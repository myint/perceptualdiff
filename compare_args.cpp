/*
Compare Args
Copyright (C) 2006-2011 Yangli Hector Yee
Copyright (C) 2011-2015 Steven Myint, Jeff Terrace

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

#include "rgba_image.h"

#include <cassert>
#include <ciso646>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <sstream>


static const auto VERSION = "1.2";


static const auto USAGE = "Usage: peceptualdiff image1 image2\n\
\n\
Compares image1 and image2 using a perceptually based image metric.\n\
\n\
Options:\n\
  --verbose        Turn on verbose mode\n\
  --fov deg        Field of view in degrees [0.1, 89.9] (default: 45.0)\n\
  --threshold p    Number of pixels p below which differences are ignored\n\
  --gamma g        Value to convert rgb into linear space (default: 2.2)\n\
  --luminance l    White luminance (default: 100.0 cdm^-2)\n\
  --luminanceonly  Only consider luminance; ignore chroma (color) in the\n\
                   comparison\n\
  --colorfactor    How much of color to use [0.0, 1.0] (default: 1.0)\n\
  --downsample     How many powers of two to down sample the image\n\
                   (default: 0)\n\
  --scale          Scale images to match each other's dimensions\n\
  --sum-errors     Print a sum of the luminance and color differences\n\
  --output o       Write difference to the file o\n\
  --version        Print version\n\
\n";


template <typename T>
static T lexical_cast(const std::string &input)
{
    std::stringstream ss(input);
    T output;
    if (not (ss >> output))
    {
        throw std::invalid_argument("");
    }
    return output;
}


static bool option_matches(const char *arg, const std::string &option_name)
{
    const auto string_arg = std::string(arg);

    return (string_arg == "--" + option_name) or
           (string_arg == "-" + option_name);
}


CompareArgs::CompareArgs()
{
    verbose_ = false;
    luminance_only_ = false;
    sum_errors_ = false;
    field_of_view_ = 45.0f;
    gamma_ = 2.2f;
    threshold_pixels_ = 100;
    luminance_ = 100.0f;
    color_factor_ = 1.0f;
    down_sample_ = 0;
}


static void print_help()
{
    std::cout << USAGE;
    std::cout << std::endl;
}

bool CompareArgs::parse_args(const int argc, char **argv)
{
    if (argc <= 1)
    {
        print_help();
        return false;
    }

    auto image_count = 0u;
    const char *output_file_name = nullptr;
    auto scale = false;
    for (auto i = 1; i < argc; i++)
    {
        try
        {
            if (option_matches(argv[i], "help"))
            {
                print_help();
                exit(EXIT_SUCCESS);
            }
            else if (option_matches(argv[i], "fov"))
            {
                if (++i < argc)
                {
                    field_of_view_ = lexical_cast<float>(argv[i]);
                }
            }
            else if (option_matches(argv[i], "verbose"))
            {
                verbose_ = true;
            }
            else if (option_matches(argv[i], "threshold"))
            {
                if (++i < argc)
                {
                    auto temporary = lexical_cast<int>(argv[i]);
                    if (temporary < 0)
                    {
                        throw std::invalid_argument(
                            "-threshold must be positive");
                    }
                    threshold_pixels_ = static_cast<unsigned int>(temporary);
                }
            }
            else if (option_matches(argv[i], "gamma"))
            {
                if (++i < argc)
                {
                    gamma_ = lexical_cast<float>(argv[i]);
                }
            }
            else if (option_matches(argv[i], "luminance"))
            {
                if (++i < argc)
                {
                    luminance_ = lexical_cast<float>(argv[i]);
                }
            }
            else if (option_matches(argv[i], "luminanceonly"))
            {
                luminance_only_ = true;
            }
            else if (option_matches(argv[i], "sum-errors"))
            {
                sum_errors_ = true;
            }
            else if (option_matches(argv[i], "colorfactor"))
            {
                if (++i < argc)
                {
                    color_factor_ = lexical_cast<float>(argv[i]);
                }
            }
            else if (option_matches(argv[i], "downsample"))
            {
                if (++i < argc)
                {
                    auto temporary = lexical_cast<int>(argv[i]);
                    if (temporary < 0)
                    {
                        throw std::invalid_argument(
                            "--downsample must be positive");
                    }
                    down_sample_ = static_cast<unsigned int>(temporary);
                    assert(down_sample_ <= INT_MAX);
                }
            }
            else if (option_matches(argv[i], "scale"))
            {
                scale = true;
            }
            else if (option_matches(argv[i], "output"))
            {
                if (++i < argc)
                {
                    output_file_name = argv[i];
                }
            }
            else if (option_matches(argv[i], "version"))
            {
                std::cout << "perceptualdiff " << VERSION << std::endl;
                exit(EXIT_SUCCESS);
            }
            else if (image_count < 2)
            {
                auto image = read_from_file(argv[i]);

                ++image_count;
                if (image_count == 1)
                {
                    image_a_ = image;
                }
                else
                {
                    image_b_ = image;
                }
            }
            else
            {
                std::cerr << "Warning: option/file \"" << argv[i]
                          << "\" ignored\n";
            }
        }
        catch (const std::invalid_argument &exception)
        {
            std::string reason = "";
            if (not std::string(exception.what()).empty())
            {
                reason = std::string("; ") + exception.what();
            }
            throw ParseException("Invalid argument (" + std::string(argv[i]) +
                                 ") for " + argv[i - 1] + reason);
        }
    }

    if (not image_a_ or not image_b_)
    {
        error_string_ = "Not enough image files specified\n";
        return false;
    }

    for (auto i = 0u; i < down_sample_; i++)
    {
        const auto tmp_a = image_a_->down_sample();
        const auto tmp_b = image_b_->down_sample();

        if (tmp_a and tmp_b)
        {
            image_a_ = tmp_a;
            image_b_ = tmp_b;
        }
        else
        {
            break;
        }

        if (verbose_)
        {
            std::cout << "Downsampling by " << (1 << (i + 1)) << "\n";
        }
    }

    if (scale and
        (image_a_->get_width() != image_b_->get_width() or
         image_a_->get_height() != image_b_->get_height()))
    {
        auto min_width = image_a_->get_width();
        if (image_b_->get_width() < min_width)
        {
            min_width = image_b_->get_width();
        }

        auto min_height = image_a_->get_height();
        if (image_b_->get_height() < min_height)
        {
            min_height = image_b_->get_height();
        }

        if (verbose_)
        {
            std::cout << "Scaling to " << min_width << " x " << min_height
                      << "\n";
        }
        auto tmp = image_a_->down_sample(min_width, min_height);
        if (tmp)
        {
            image_a_ = tmp;
        }
        tmp = image_b_->down_sample(min_width, min_height);
        if (tmp)
        {
            image_b_ = tmp;
        }
    }
    if (output_file_name)
    {
        image_difference_ = std::make_shared<RGBAImage>(image_a_->get_width(),
                                                        image_a_->get_height(),
                                                        output_file_name);
    }
    return true;
}

void CompareArgs::print_args() const
{
    std::cout << "Field of view is " << field_of_view_ << " degrees\n"
              << "Threshold pixels is " << threshold_pixels_ << " pixels\n"
              << "The gamma is " << gamma_ << "\n"
              << "The display's luminance is " << luminance_
              << " candela per meter squared\n";
}
