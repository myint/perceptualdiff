/*
Laplacian Pyramid
Copyright (C) 2006 Yangli Hector Yee

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


LPyramid::LPyramid(const float *image, unsigned int width, unsigned int height) :
	Width(width),
	Height(height)
{
	// Make the Laplacian pyramid by successively
	// copying the earlier levels and blurring them
	for (unsigned int i = 0; i < MAX_PYR_LEVELS; i++) {
		if (i == 0) {
			Levels[i] = Copy(image);
		} else {
			Levels[i] = new float[Width * Height];
			Convolve(Levels[i], Levels[i - 1]);
		}
	}
}

LPyramid::~LPyramid()
{
	for (unsigned int i = 0; i < MAX_PYR_LEVELS; i++) {
		delete [] Levels[i];
	}
}

float *LPyramid::Copy(const float *img) const
{
	unsigned int max = Width * Height;
	float *out = new float[max];
	for (unsigned int i = 0; i < max; i++) {
		out[i] = img[i];
	}

	return out;
}

void LPyramid::Convolve(float *a, const float *b) const
// convolves image b with the filter kernel and stores it in a
{
	const float Kernel[] = {0.05f, 0.25f, 0.4f, 0.25f, 0.05f};
	#pragma omp parallel for
	for (unsigned int y = 0; y < Height; y++) {
		for (unsigned int x = 0; x < Width; x++) {
			unsigned int index = y * Width + x;
			a[index] = 0.0f;
			for (int i = -2; i <= 2; i++) {
				for (int j = -2; j <= 2; j++) {
					int nx = x + i;
					int ny = y + j;
					if (nx < 0) {
						nx = -nx;
					}
					if (ny < 0) {
						ny = -ny;
					}
					if (nx >= static_cast<long>(Width)) {
						nx = 2 * Width - nx - 1;
					}
					if (ny >= static_cast<long>(Height)) {
						ny = 2 * Height - ny - 1;
					}
					a[index] += Kernel[i + 2] * Kernel[j + 2] * b[ny * Width + nx];
				}
			}
		}
	}
}

float LPyramid::Get_Value(unsigned int x, unsigned int y, unsigned int level) const
{
	unsigned int index = x + y * Width;
	unsigned int l = level;
	assert(l < MAX_PYR_LEVELS);
	return Levels[level][index];
}
