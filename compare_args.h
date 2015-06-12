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

#ifndef PERCEPTUALDIFF_COMPARE_ARGS_H
#define PERCEPTUALDIFF_COMPARE_ARGS_H

#include <memory>
#include <stdexcept>
#include <string>


class RGBAImage;


// Arguments to pass into the comparison function.
class CompareArgs
{
public:

    CompareArgs();

    bool parse_args(int argc, const char **argv);
    void print_args() const;

    std::shared_ptr<RGBAImage> image_a_;
    std::shared_ptr<RGBAImage> image_b_;
    std::shared_ptr<RGBAImage> image_difference_;
    bool verbose_;

    // Only consider luminance; ignore chroma channels in the comparison.
    bool luminance_only_;

    // Print a sum of the luminance and color differences of each pixel.
    bool sum_errors_;

    // Field of view in degrees.
    float field_of_view_;

    // The gamma to convert to linear color space
    float gamma_;

    float luminance_;

    // How many pixels different to ignore.
    unsigned int threshold_pixels_;

    std::string error_string_;

    // How much color to use in the metric.
    // 0.0 is the same as luminance_only_ = true,
    // 1.0 means full strength.
    float color_factor_;

    // How much to down sample image before comparing, in powers of 2.
    unsigned int down_sample_;
};


class ParseException : public virtual std::invalid_argument
{
public:

    ParseException(const std::string &message)
        : std::invalid_argument(message)
    {
    }
};

#endif
