/*
Laplacian Pyramid
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

#include <cassert>

#include "LPyramid.h"


static std::vector<float> Copy(const float *img,
                               const unsigned int width,
                               const unsigned int height)
{
    const auto max = width * height;
    std::vector<float> out(max);
    for (auto i = 0u; i < max; i++)
    {
        out[i] = img[i];
    }

    return out;
}


LPyramid::LPyramid(const float *image,
                   const unsigned int width, const unsigned int height)
    : width_(width), weight_(height)
{
    // Make the Laplacian pyramid by successively
    // copying the earlier levels and blurring them
    for (auto i = 0u; i < MAX_PYR_LEVELS; i++)
    {
        if (i == 0 or width * height <= 1)
        {
            Levels[i] = Copy(image, width, height);
        }
        else
        {
            Levels[i].resize(width_ * weight_);
            convolve(Levels[i], Levels[i - 1]);
        }
    }
}

// Convolves image b with the filter kernel and stores it in a.
void LPyramid::convolve(std::vector<float> &a,
                        const std::vector<float> &b) const
{
    assert(a.size() > 1);
    assert(b.size() > 1);

    const float Kernel[] = {0.05f, 0.25f, 0.4f, 0.25f, 0.05f};
#pragma omp parallel for
    for (auto y = 0u; y < weight_; y++)
    {
        for (auto x = 0u; x < width_; x++)
        {
            auto index = y * width_ + x;
            a[index] = 0.0f;
            for (auto i = -2; i <= 2; i++)
            {
                for (auto j = -2; j <= 2; j++)
                {
                    int nx = x + i;
                    int ny = y + j;
                    if (nx < 0)
                    {
                        nx = -nx;
                    }
                    if (ny < 0)
                    {
                        ny = -ny;
                    }
                    if (nx >= static_cast<long>(width_))
                    {
                        nx = 2 * width_ - nx - 1;
                    }
                    if (ny >= static_cast<long>(weight_))
                    {
                        ny = 2 * weight_ - ny - 1;
                    }
                    a[index] +=
                        Kernel[i + 2] * Kernel[j + 2] * b[ny * width_ + nx];
                }
            }
        }
    }
}

float LPyramid::get_value(const unsigned int x, const unsigned int y,
                          const unsigned int level) const
{
    const auto index = x + y * width_;
    assert(level < MAX_PYR_LEVELS);
    return Levels[level][index];
}
