/*
RGBAImage.cpp
Copyright (C) 2006 Yangli Hector Yee

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
#include <cstdio>
#include <cstring>
#include <cassert>


static FIBITMAP *ToFreeImage(const RGBAImage &image)
{
	const unsigned int *data = image.Get_Data();

	FIBITMAP *bitmap = FreeImage_Allocate(image.Get_Width(),
	                                      image.Get_Height(),
	                                      32,
	                                      0x000000ff,
	                                      0x0000ff00,
	                                      0x00ff0000);
	assert(bitmap);

	for (int y=0; y < image.Get_Height(); y++, data += image.Get_Width())
	{
		unsigned int *scanline = reinterpret_cast<unsigned int*>(FreeImage_GetScanLine(bitmap, image.Get_Height() - y - 1));
		memcpy(scanline, data, sizeof(data[0]) * image.Get_Width());
	}

	return bitmap;
}


static RGBAImage *ToRGBAImage(FIBITMAP *image, const char *filename=NULL)
{
	const int w = FreeImage_GetWidth(image);
	const int h = FreeImage_GetHeight(image);

	RGBAImage *result = new RGBAImage(w, h, filename);
	// Copy the image over to our internal format, FreeImage has the scanlines bottom to top though.
	unsigned int *dest = result->Get_Data();
	for ( int y=0; y < h; y++, dest += w )
	{
		const unsigned int *scanline = reinterpret_cast<const unsigned int*>(FreeImage_GetScanLine(image, h - y - 1));
		memcpy(dest, scanline, sizeof(dest[0]) * w);
	}

	return result;
}


RGBAImage *RGBAImage::DownSample(int w, int h) const {
	if (w == 0)
	{
		w = Width / 2;
	}

	if (h == 0)
	{
		h = Height / 2;
	}

	if (Width <= 1 || Height <= 1) return NULL;
	if (Width == w && Height == h) return NULL;
	assert(w <= Width);
	assert(h <= Height);

	FIBITMAP *bitmap = ToFreeImage(*this);
	FIBITMAP *converted = FreeImage_Rescale(bitmap, w, h, FILTER_BICUBIC);

	FreeImage_Unload(bitmap);
	bitmap = NULL;

	RGBAImage *img = ToRGBAImage(converted, Name.c_str());

	FreeImage_Unload(converted);

	return img;
}

bool RGBAImage::WriteToFile(const char *filename) const
{
	const FREE_IMAGE_FORMAT fileType = FreeImage_GetFIFFromFilename(filename);
	if (FIF_UNKNOWN == fileType)
	{
		printf("Can't save to unknown filetype %s\n", filename);
		return false;
	}

	FIBITMAP *bitmap = ToFreeImage(*this);

	FreeImage_SetTransparent(bitmap, false);
	FIBITMAP *converted = FreeImage_ConvertTo24Bits(bitmap);

	const bool result = !!FreeImage_Save(fileType, converted, filename);
	if (!result)
		printf("Failed to save to %s\n", filename);

	FreeImage_Unload(converted);
	FreeImage_Unload(bitmap);
	return result;
}

RGBAImage *RGBAImage::ReadFromFile(const char *filename)
{
	const FREE_IMAGE_FORMAT fileType = FreeImage_GetFileType(filename);
	if (FIF_UNKNOWN == fileType)
	{
		printf("Unknown filetype %s\n", filename);
		return 0;
	}

	FIBITMAP *freeImage = 0;
	if (FIBITMAP *temporary = FreeImage_Load(fileType, filename, 0))
	{
		freeImage = FreeImage_ConvertTo32Bits(temporary);
		FreeImage_Unload(temporary);
	}
	if (!freeImage)
	{
		printf( "Failed to load the image %s\n", filename);
		return 0;
	}

	RGBAImage *result = ToRGBAImage(freeImage);

	FreeImage_Unload(freeImage);

	return result;
}
