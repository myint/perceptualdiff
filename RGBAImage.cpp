/*
RGBAImage.cpp
Copyright (C) 2006-2011 Yangli Hector Yee
Copyright (C) 2011-2014 Steven Myint

(This entire file was rewritten by Jim Tilander)

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

#include "RGBAImage.h"
#include "FreeImage.h"

#include <iostream>
#include <string>
#include <cstring>
#include <cassert>


static FIBITMAP *ToFreeImage(const RGBAImage &image)
{
    const auto *data = image.Get_Data();

    auto bitmap = FreeImage_Allocate(image.Get_Width(), image.Get_Height(), 32,
                                     0x000000ff, 0x0000ff00, 0x00ff0000);
    assert(bitmap);

    for (auto y = 0u; y < image.Get_Height();
         y++, data += image.Get_Width())
    {
        auto scanline = reinterpret_cast<unsigned int *>(
            FreeImage_GetScanLine(bitmap, image.Get_Height() - y - 1));
        memcpy(scanline, data, sizeof(data[0]) * image.Get_Width());
    }

    return bitmap;
}


static std::shared_ptr<RGBAImage> ToRGBAImage(FIBITMAP *image,
                                              const std::string &filename = "")
{
    const auto w = FreeImage_GetWidth(image);
    const auto h = FreeImage_GetHeight(image);

    auto result = std::make_shared<RGBAImage>(w, h, filename);
    // Copy the image over to our internal format, FreeImage has the scanlines
    // bottom to top though.
    auto dest = result->Get_Data();
    for (unsigned int y = 0; y < h; y++, dest += w)
    {
        const auto scanline = reinterpret_cast<const unsigned int *>(
            FreeImage_GetScanLine(image, h - y - 1));
        memcpy(dest, scanline, sizeof(dest[0]) * w);
    }

    return result;

}

std::shared_ptr<RGBAImage> RGBAImage::DownSample(unsigned int w,
                                                 unsigned int h) const
{
    if (w == 0)
    {
        w = Width / 2;
    }

    if (h == 0)
    {
        h = Height / 2;
    }

    if (Width <= 1 or Height <= 1)
    {
        return nullptr;
    }
    if (Width == w and Height == h)
    {
        return nullptr;
    }
    assert(w <= Width);
    assert(h <= Height);

    auto bitmap = ToFreeImage(*this);
    auto converted = FreeImage_Rescale(bitmap, w, h, FILTER_BICUBIC);

    FreeImage_Unload(bitmap);
    bitmap = nullptr;

    auto img = ToRGBAImage(converted, Name);

    FreeImage_Unload(converted);

    return img;
}

bool RGBAImage::WriteToFile(const std::string &filename) const
{
    const auto fileType = FreeImage_GetFIFFromFilename(filename.c_str());
    if (FIF_UNKNOWN == fileType)
    {
        std::cerr << "Can't save to unknown filetype " << filename
                  << std::endl;
        return false;
    }

    auto bitmap = ToFreeImage(*this);

    FreeImage_SetTransparent(bitmap, false);
    auto converted = FreeImage_ConvertTo24Bits(bitmap);

    const bool result =
        !!FreeImage_Save(fileType, converted, filename.c_str());
    if (not result)
    {
        std::cerr << "Failed to save to " << filename << std::endl;
    }

    FreeImage_Unload(converted);
    FreeImage_Unload(bitmap);
    return result;
}

std::shared_ptr<RGBAImage> RGBAImage::ReadFromFile(const std::string &filename)
{
    const auto fileType = FreeImage_GetFileType(filename.c_str());
    if (FIF_UNKNOWN == fileType)
    {
        std::cerr << "Unknown filetype " << filename << std::endl;
        return nullptr;
    }

    FIBITMAP *freeImage = nullptr;
    if (auto temporary = FreeImage_Load(fileType, filename.c_str(), 0))
    {
        freeImage = FreeImage_ConvertTo32Bits(temporary);
        FreeImage_Unload(temporary);
    }
    if (not freeImage)
    {
        std::cerr << "Failed to load the image " << filename << std::endl;
        return nullptr;
    }

    auto result = ToRGBAImage(freeImage);

    FreeImage_Unload(freeImage);

    return result;
}
