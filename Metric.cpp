/*
Metric
Copyright (C) 2006-2011 Yangli Hector Yee
Copyright (C) 2011-2014 Steven Myint

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

#include "Metric.h"
#include "CompareArgs.h"
#include "RGBAImage.h"
#include "LPyramid.h"

#include <cmath>
#include <iostream>
#include <memory>


#ifndef M_PI
#define M_PI 3.14159265f
#endif


/*
* Given the adaptation luminance, this function returns the
* threshold of visibility in cd per m^2
* TVI means Threshold vs Intensity function
* This version comes from Ward Larson Siggraph 1997
*/
static float tvi(float adaptation_luminance)
{
    // returns the threshold luminance given the adaptation luminance
    // units are candelas per meter squared

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
static float csf(float cpd, float lum)
{
    const auto a = 440.f * powf((1.f + 0.7f / lum), -0.2f);
    const auto b = 0.3f * powf((1.0f + 100.0f / lum), 0.15f);

    return a * cpd * expf(-b * cpd) * sqrtf(1.0f + 0.06f * expf(b * cpd));
}


/*
* Visual Masking Function
* from Daly 1993
*/
static float mask(float contrast)
{
    const auto a = powf(392.498f * contrast, 0.7f);
    const auto b = powf(0.0153f * a, 4.f);
    return powf(1.0f + b, 0.25f);
}


// convert Adobe RGB (1998) with reference white D65 to XYZ
static void AdobeRGBToXYZ(float r, float g, float b,
                          float &x, float &y, float &z)
{
    // matrix is from http://www.brucelindbloom.com/
    x = r * 0.576700f + g * 0.185556f + b * 0.188212f;
    y = r * 0.297361f + g * 0.627355f + b * 0.0752847f;
    z = r * 0.0270328f + g * 0.0706879f + b * 0.991248f;
}


struct White
{
    White()
    {
        AdobeRGBToXYZ(1.f, 1.f, 1.f, x, y, z);
    }

    float x;
    float y;
    float z;
};


static const White global_white;


static void XYZToLAB(float x, float y, float z, float &L, float &A, float &B)
{
    const float epsilon = 216.0f / 24389.0f;
    const float kappa = 24389.0f / 27.0f;
    float f[3];
    float r[3];
    r[0] = x / global_white.x;
    r[1] = y / global_white.y;
    r[2] = z / global_white.z;
    for (unsigned int i = 0; i < 3; i++)
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


static unsigned int adaptation(float num_one_degree_pixels)
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


bool Yee_Compare(CompareArgs &args)
{
    if ((args.ImgA->Get_Width() != args.ImgB->Get_Width())or(
            args.ImgA->Get_Height() != args.ImgB->Get_Height()))
    {
        args.ErrorStr = "Image dimensions do not match\n";
        return false;
    }

    const auto dim = args.ImgA->Get_Width() * args.ImgA->Get_Height();
    auto identical = true;
    for (auto i = 0u; i < dim; i++)
    {
        if (args.ImgA->Get(i) != args.ImgB->Get(i))
        {
            identical = false;
            break;
        }
    }
    if (identical)
    {
        args.ErrorStr = "Images are binary identical\n";
        return true;
    }

    // assuming colorspaces are in Adobe RGB (1998) convert to XYZ
    auto aX = std::unique_ptr<float[]>(new float[dim]);
    auto aY = std::unique_ptr<float[]>(new float[dim]);
    auto aZ = std::unique_ptr<float[]>(new float[dim]);
    auto bX = std::unique_ptr<float[]>(new float[dim]);
    auto bY = std::unique_ptr<float[]>(new float[dim]);
    auto bZ = std::unique_ptr<float[]>(new float[dim]);
    auto aLum = std::unique_ptr<float[]>(new float[dim]);
    auto bLum = std::unique_ptr<float[]>(new float[dim]);

    auto aA = std::unique_ptr<float[]>(new float[dim]);
    auto bA = std::unique_ptr<float[]>(new float[dim]);
    auto aB = std::unique_ptr<float[]>(new float[dim]);
    auto bB = std::unique_ptr<float[]>(new float[dim]);

    if (args.Verbose)
    {
        std::cout << "Converting RGB to XYZ\n";
    }

    const auto w = args.ImgA->Get_Width();
    const auto h = args.ImgA->Get_Height();
#pragma omp parallel for
    for (auto y = 0u; y < h; y++)
    {
        for (auto x = 0u; x < w; x++)
        {
            const auto i = x + y * w;
            auto r = powf(args.ImgA->Get_Red(i) / 255.0f, args.Gamma);
            auto g = powf(args.ImgA->Get_Green(i) / 255.0f, args.Gamma);
            auto b = powf(args.ImgA->Get_Blue(i) / 255.0f, args.Gamma);
            AdobeRGBToXYZ(r, g, b, aX[i], aY[i], aZ[i]);
            float l;
            XYZToLAB(aX[i], aY[i], aZ[i], l, aA[i], aB[i]);
            r = powf(args.ImgB->Get_Red(i) / 255.0f, args.Gamma);
            g = powf(args.ImgB->Get_Green(i) / 255.0f, args.Gamma);
            b = powf(args.ImgB->Get_Blue(i) / 255.0f, args.Gamma);
            AdobeRGBToXYZ(r, g, b, bX[i], bY[i], bZ[i]);
            XYZToLAB(bX[i], bY[i], bZ[i], l, bA[i], bB[i]);
            aLum[i] = aY[i] * args.Luminance;
            bLum[i] = bY[i] * args.Luminance;
        }
    }

    if (args.Verbose)
    {
        std::cout << "Constructing Laplacian Pyramids\n";
    }

    const LPyramid la(aLum.get(), w, h);
    const LPyramid lb(bLum.get(), w, h);

    const auto num_one_degree_pixels =
        2.f * tan(args.FieldOfView * 0.5 * M_PI / 180) * 180 / M_PI;
    const auto pixels_per_degree = w / num_one_degree_pixels;

    if (args.Verbose)
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
#pragma omp parallel for reduction(+ : pixels_failed) reduction(+ : error_sum)
    for (auto y = 0u; y < h; y++)
    {
        for (auto x = 0u; x < w; x++)
        {
            const auto index = x + y * w;
            float contrast[MAX_PYR_LEVELS - 2];
            float sum_contrast = 0;
            for (auto i = 0u; i < MAX_PYR_LEVELS - 2; i++)
            {
                auto n1 =
                    fabsf(la.Get_Value(x, y, i) - la.Get_Value(x, y, i + 1));
                auto n2 =
                    fabsf(lb.Get_Value(x, y, i) - lb.Get_Value(x, y, i + 1));
                auto numerator = (n1 > n2) ? n1 : n2;
                auto d1 = fabsf(la.Get_Value(x, y, i + 2));
                auto d2 = fabsf(lb.Get_Value(x, y, i + 2));
                auto denominator = (d1 > d2) ? d1 : d2;
                if (denominator < 1e-5f)
                {
                    denominator = 1e-5f;
                }
                contrast[i] = numerator / denominator;
                sum_contrast += contrast[i];
            }
            if (sum_contrast < 1e-5)
            {
                sum_contrast = 1e-5f;
            }
            float F_mask[MAX_PYR_LEVELS - 2];
            auto adapt = la.Get_Value(x, y, adaptation_level) +
                         lb.Get_Value(x, y, adaptation_level);
            adapt *= 0.5f;
            if (adapt < 1e-5)
            {
                adapt = 1e-5f;
            }
            for (auto i = 0u; i < MAX_PYR_LEVELS - 2; i++)
            {
                F_mask[i] = mask(contrast[i] * csf(cpd[i], adapt));
            }
            auto factor = 0.f;
            for (auto i = 0u; i < MAX_PYR_LEVELS - 2; i++)
            {
                factor += contrast[i] * F_freq[i] * F_mask[i] / sum_contrast;
            }
            if (factor < 1)
            {
                factor = 1;
            }
            if (factor > 10)
            {
                factor = 10;
            }
            const auto delta =
                fabsf(la.Get_Value(x, y, 0) - lb.Get_Value(x, y, 0));
            error_sum += delta;
            auto pass = true;

            // pure luminance test
            if (delta > factor * tvi(adapt))
            {
                pass = false;
            }

            if (not args.LuminanceOnly)
            {
                // CIE delta E test with modifications
                auto color_scale = args.ColorFactor;
                // ramp down the color test in scotopic regions
                if (adapt < 10.0f)
                {
                    // Don't do color test at all.
                    color_scale = 0.0;
                }
                auto da = aA[index] - bA[index];
                auto db = aB[index] - bB[index];
                da = da * da;
                db = db * db;
                const auto delta_e = (da + db) * color_scale;
                error_sum += delta_e;
                if (delta_e > factor)
                {
                    pass = false;
                }
            }

            if (not pass)
            {
                pixels_failed++;
                if (args.ImgDiff)
                {
                    args.ImgDiff->Set(255, 0, 0, 255, index);
                }
            }
            else
            {
                if (args.ImgDiff)
                {
                    args.ImgDiff->Set(0, 0, 0, 255, index);
                }
            }
        }
    }

    const auto error_sum_buff =
        std::to_string(error_sum) + " error sum\n";

    const auto different =
        std::to_string(pixels_failed) + " pixels are different\n";

    // Always output image difference if requested.
    if (args.ImgDiff)
    {
        if (args.ImgDiff->WriteToFile(args.ImgDiff->Get_Name()))
        {
            args.ErrorStr += "Wrote difference image to ";
            args.ErrorStr += args.ImgDiff->Get_Name();
            args.ErrorStr += "\n";
        }
        else
        {
            args.ErrorStr += "Could not write difference image to ";
            args.ErrorStr += args.ImgDiff->Get_Name();
            args.ErrorStr += "\n";
        }
    }

    if (pixels_failed < args.ThresholdPixels)
    {
        args.ErrorStr = "Images are perceptually indistinguishable\n";
        args.ErrorStr += different;
        return true;
    }

    args.ErrorStr = "Images are visibly different\n";
    args.ErrorStr += different;
    if (args.SumErrors)
    {
        args.ErrorStr += error_sum_buff;
    }

    return false;
}
