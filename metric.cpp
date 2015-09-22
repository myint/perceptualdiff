/*
Metric
Copyright (C) 2006-2011 Yangli Hector Yee
Copyright (C) 2011-2015 Steven Myint, Jeff Terrace

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "metric.h"

#include "compare_args.h"
#include "lpyramid.h"
#include "rgba_image.h"
#include "dispatch_wrapper.h"

#include <ciso646>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <vector>
#include <algorithm>


#if _MSC_VER <= 1800
static const auto pi = 3.14159265f;


static float to_radians(const float degrees)  // LCOV_EXCL_LINE
{
  return degrees * pi / 180.f;  // LCOV_EXCL_LINE
}


static float to_degrees(const float radians)  // LCOV_EXCL_LINE
{
  return radians * 180.f / pi;  // LCOV_EXCL_LINE
}
#else
constexpr auto pi = 3.14159265f;


constexpr float to_radians(const float degrees)  // LCOV_EXCL_LINE
{
    return degrees * pi / 180.f;  // LCOV_EXCL_LINE
}


constexpr float to_degrees(const float radians)  // LCOV_EXCL_LINE
{
    return radians * 180.f / pi;  // LCOV_EXCL_LINE
}
#endif

// Given the adaptation luminance, this function returns the
// threshold of visibility in cd per m^2.
//
// TVI means Threshold vs Intensity function.
// This version comes from Ward Larson Siggraph 1997.
//
// Returns the threshold luminance given the adaptation luminance.
// Units are candelas per meter squared.
static float tvi(const float adaptation_luminance)
{
    const auto log_a = log10f(adaptation_luminance);

    float r;
    if (log_a < -3.94f)
    {
        r = -2.86f;
    }
    else if (log_a < -1.44f)
    {
        r = powf(0.405f * log_a + 1.6f, 2.18f) - 2.86f;
    }
    else if (log_a < -0.0184f)
    {
        r = log_a - 0.395f;
    }
    else if (log_a < 1.9f)
    {
        r = powf(0.249f * log_a + 0.65f, 2.7f) - 0.72f;
    }
    else
    {
        r = log_a - 1.255f;
    }

    return powf(10.0f, r);
}


// computes the contrast sensitivity function (Barten SPIE 1989)
// given the cycles per degree (cpd) and luminance (lum)
static float csf(const float cpd, const float lum)
{
    const auto a = 440.f * powf((1.f + 0.7f / lum), -0.2f);
    const auto b = 0.3f * powf((1.0f + 100.0f / lum), 0.15f);

    return a * cpd * expf(-b * cpd) * sqrtf(1.0f + 0.06f * expf(b * cpd));
}


/*
* Visual Masking Function
* from Daly 1993
*/
static float mask(const float contrast)
{
    const auto a = powf(392.498f * contrast, 0.7f);
    const auto b = powf(0.0153f * a, 4.f);
    return powf(1.0f + b, 0.25f);
}


// convert Adobe RGB (1998) with reference white D65 to XYZ
static void adobe_rgb_to_xyz(const float r, const float g, const float b,
                             float &x, float &y, float &z)
{
    // matrix is from http://www.brucelindbloom.com/
    x = r * 0.576700f  + g * 0.185556f  + b * 0.188212f;
    y = r * 0.297361f  + g * 0.627355f  + b * 0.0752847f;
    z = r * 0.0270328f + g * 0.0706879f + b * 0.991248f;
}


struct White
{
    White()
    {
        adobe_rgb_to_xyz(1.f, 1.f, 1.f, x, y, z);
    }

    float x;
    float y;
    float z;
};


static const White global_white;


static void xyz_to_lab(const float x, const float y, const float z, float &L, float &A, float &B)
{
    const float epsilon = 216.0f / 24389.0f;
    const float kappa = 24389.0f / 27.0f;
    const float r[] = {
        x / global_white.x,
        y / global_white.y,
        z / global_white.z
    };
    float f[3];
    for (auto i = 0u; i < 3; i++)
    {
        if (r[i] > epsilon)
        {
            f[i] = powf(r[i], 1.0f / 3.0f);
        }
        else
        {
            f[i] = (kappa * r[i] + 16.0f) / 116.0f;
        }
    }
    L = 116.0f * f[1] - 16.0f;
    A = 500.0f * (f[0] - f[1]);
    B = 200.0f * (f[1] - f[2]);
}


static unsigned int adaptation(const float num_one_degree_pixels)
{
    auto num_pixels = 1.f;
    auto adaptation_level = 0u;
    for (auto i = 0u; i < MAX_PYR_LEVELS; i++)
    {
        adaptation_level = i;
        if (num_pixels > num_one_degree_pixels)
        {
            break;
        }
        num_pixels *= 2;
    }
    return adaptation_level;  // LCOV_EXCL_LINE
}

bool yee_compare(CompareArgs &args)
{
    if ((args.image_a_->get_width()  != args.image_b_->get_width()) or
        (args.image_a_->get_height() != args.image_b_->get_height()))
    {
        args.error_string_ = "Image dimensions do not match\n";
        return false;
    }

    const auto w = args.image_a_->get_width();
    const auto h = args.image_a_->get_height();
    const auto dim = w * h;

    auto identical = true;
    for (auto i = 0u; i < dim; i++)
    {
        if (args.image_a_->get(i) != args.image_b_->get(i))
        {
            identical = false;
            break;
        }
    }
    if (identical)
    {
        args.error_string_ = "Images are binary identical\n";
        return true;
    }

    // Assuming colorspaces are in Adobe RGB (1998) convert to XYZ.
    std::vector<float> a_lum(dim);
    std::vector<float> b_lum(dim);

    std::vector<float> a_a(dim);
    std::vector<float> b_a(dim);
    std::vector<float> a_b(dim);
    std::vector<float> b_b(dim);

    if (args.verbose_)
    {
        std::cout << "Converting RGB to XYZ\n";
    }

    const auto gamma = args.gamma_;
    const auto luminance = args.luminance_;

    dispatch::dispatch_apply(static_cast<ptrdiff_t>(h), dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), [&] (size_t y) {
        for (auto x = 0u; x < w; x++)
        {
            const auto i = x + y * w;
            const auto a_color_r = powf(args.image_a_->get_red(i) / 255.0f, gamma);
            const auto a_color_g = powf(args.image_a_->get_green(i) / 255.0f, gamma);
            const auto a_color_b = powf(args.image_a_->get_blue(i) / 255.0f, gamma);
            float a_x;
            float a_y;
            float a_z;
            adobe_rgb_to_xyz(a_color_r, a_color_g, a_color_b, a_x, a_y, a_z);
            float l;
            xyz_to_lab(a_x, a_y, a_z, l, a_a[i], a_b[i]);
            const auto b_color_r = powf(args.image_b_->get_red(i) / 255.0f, gamma);
            const auto b_color_g = powf(args.image_b_->get_green(i) / 255.0f, gamma);
            const auto b_color_b = powf(args.image_b_->get_blue(i) / 255.0f, gamma);
            float b_x;
            float b_y;
            float b_z;
            adobe_rgb_to_xyz(b_color_r, b_color_g, b_color_b, b_x, b_y, b_z);
            xyz_to_lab(b_x, b_y, b_z, l, b_a[i], b_b[i]);
            a_lum[i] = a_y * luminance;
            b_lum[i] = b_y * luminance;
        }
    });

    if (args.verbose_)
    {
        std::cout << "Constructing Laplacian Pyramids\n";
    }

    const LPyramid la(a_lum, w, h);
    const LPyramid lb(b_lum, w, h);

    const auto num_one_degree_pixels =
        to_degrees(2 *
                   std::tan(args.field_of_view_ * to_radians(.5f)));
    const auto pixels_per_degree = w / num_one_degree_pixels;

    if (args.verbose_)
    {
        std::cout << "Performing test\n";
    }

    const auto adaptation_level = adaptation(num_one_degree_pixels);

    float cpd[MAX_PYR_LEVELS];
    cpd[0] = 0.5f * pixels_per_degree;
    for (auto i = 1u; i < MAX_PYR_LEVELS; i++)
    {
        cpd[i] = 0.5f * cpd[i - 1];
    }
    const auto csf_max = csf(3.248f, 100.0f);

    static_assert(MAX_PYR_LEVELS > 2,
                  "MAX_PYR_LEVELS must be greater than 2");

    float F_freq[MAX_PYR_LEVELS - 2];
    for (auto i = 0u; i < MAX_PYR_LEVELS - 2; i++)
    {
        F_freq[i] = csf_max / csf(cpd[i], 100.0f);
    }

    auto pixels_failed = 0u;
    auto error_sum = 0.;

    dispatch_queue_t queue = dispatch_queue_create("", DISPATCH_QUEUE_SERIAL);

    const ptrdiff_t stride = 60;

    dispatch::dispatch_apply(static_cast<ptrdiff_t>(h) / stride + 1, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), [&] (size_t idx) {
        for (auto y = idx * stride; y < std::min(static_cast<ptrdiff_t>((idx + 1) * stride), static_cast<ptrdiff_t>(h)); y++)
        {
            auto pri_pixels_failed = 0u;
            auto pri_error_sum = 0.;
            for (auto x = 0u; x < w; x++)
            {
                const auto index = y * w + x;
                const auto adapt = std::max((la.get_value(x, y, adaptation_level) +
                                             lb.get_value(x, y, adaptation_level)) * 0.5f,
                                            1e-5f);
                auto sum_contrast = 0.f;
                auto factor = 0.f;
                for (auto i = 0u; i < MAX_PYR_LEVELS - 2; i++)
                {
                    const auto n1 =
                        fabsf(la.get_value(x, y, i) - la.get_value(x, y, i + 1));
                    const auto n2 =
                        fabsf(lb.get_value(x, y, i) - lb.get_value(x, y, i + 1));
                    const auto numerator = std::max(n1, n2);
                    const auto d1 = fabsf(la.get_value(x, y, i + 2));
                    const auto d2 = fabsf(lb.get_value(x, y, i + 2));
                    const auto denominator = std::max(std::max(d1, d2), 1e-5f);
                    const auto contrast = numerator / denominator;
                    const auto F_mask = mask(contrast * csf(cpd[i], adapt));
                    factor += contrast * F_freq[i] * F_mask;
                    sum_contrast += contrast;
                }
                sum_contrast = std::max(sum_contrast, 1e-5f);
                factor /= sum_contrast;
                factor = std::min(std::max(factor, 1.f), 10.f);
                const auto delta =
                    fabsf(la.get_value(x, y, 0) - lb.get_value(x, y, 0));

                pri_error_sum += delta;

                auto pass = true;

                // pure luminance test
                if (delta > factor * tvi(adapt))
                {
                    pass = false;
                }

                if (not args.luminance_only_)
                {
                    // CIE delta E test with modifications
                    auto color_scale = args.color_factor_;
                    // ramp down the color test in scotopic regions
                    if (adapt < 10.0f)
                    {
                        // Don't do color test at all.
                        color_scale = 0.0;
                    }
                    const auto da = a_a[index] - b_a[index];
                    const auto db = a_b[index] - b_b[index];
                    const auto delta_e = (da * da + db * db) * color_scale;

                    pri_error_sum += delta_e;

                    if (delta_e > factor)
                    {
                        pass = false;
                    }
                }

                if (not pass)
                {
                    pri_pixels_failed++;

                    if (args.image_difference_)
                    {
                        args.image_difference_->set(255, 0, 0, 255, index);
                    }
                }
                else
                {
                    if (args.image_difference_)
                    {
                        args.image_difference_->set(0, 0, 0, 255, index);
                    }
                }
            }

            dispatch::dispatch_sync(queue, [pri_error_sum, &error_sum] () { error_sum += pri_error_sum; });
            dispatch::dispatch_sync(queue, [&] () { pixels_failed += pri_pixels_failed; });
        }
    });

    dispatch_release(queue);

    const auto error_sum_buff =
        std::to_string(error_sum) + " error sum\n";

    const auto different =
        std::to_string(pixels_failed) + " pixels are different\n";

    // Always output image difference if requested.
    if (args.image_difference_)
    {
        args.image_difference_->write_to_file(args.image_difference_->get_name());

        args.error_string_ += "Wrote difference image to ";
        args.error_string_ += args.image_difference_->get_name();
        args.error_string_ += "\n";
    }

    if (pixels_failed < args.threshold_pixels_)
    {
        args.error_string_ = "Images are perceptually indistinguishable\n";
        args.error_string_ += different;
        return true;
    }

    args.error_string_ = "Images are visibly different\n";
    args.error_string_ += different;
    if (args.sum_errors_)
    {
        args.error_string_ += error_sum_buff;
    }

    return false;
}
