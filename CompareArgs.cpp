/*
Compare Args
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

#include "CompareArgs.h"
#include "RGBAImage.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <sstream>


static const char *copyright = "PerceptualDiff version 1.1.2, Copyright (C) 2006 Yangli Hector Yee\n\
PerceptualDiff comes with ABSOLUTELY NO WARRANTY;\n\
This is free software, and you are welcome\n\
to redistribute it under certain conditions;\n\
See the GPL page for details: http://www.gnu.org/copyleft/gpl.html\n\n";

static const char *usage = "Usage: peceptualdiff image1 image2\n\
\n\
Compares image1 and image2 using a perceptually based image metric.\n\
\n\
Options:\n\
    -verbose       : Turns on verbose mode\n\
    -fov deg       : Field of view in degrees (0.1 to 89.9)\n\
    -threshold p   : #pixels p below which differences are ignored\n\
    -gamma g       : Value to convert rgb into linear space (default 2.2)\n\
    -luminance l   : White luminance (default 100.0 cdm^-2)\n\
    -luminanceonly : Only consider luminance; ignore chroma (color) in the comparison\n\
    -colorfactor   : How much of color to use, 0.0 to 1.0, 0.0 = ignore color.\n\
    -downsample    : How many powers of two to down sample the image.\n\
    -scale         : Scale images to match each other's dimensions.\n\
    -sum-errors    : Print a sum of the luminance and color differences.\n\
    -output o.ppm  : Write difference to the file o.ppm\n\
\n\
Note: Input or Output files can also be in the PNG or JPG format or any format\n\
that FreeImage supports.\n";

CompareArgs::CompareArgs()
{
	ImgA = NULL;
	ImgB = NULL;
	ImgDiff = NULL;
	Verbose = false;
	LuminanceOnly = false;
	SumErrors = false;
	FieldOfView = 45.0f;
	Gamma = 2.2f;
	ThresholdPixels = 100;
	Luminance = 100.0f;
	ColorFactor = 1.0f;
	DownSample = 0;
}

CompareArgs::~CompareArgs()
{
	delete ImgA;
	delete ImgB;
	delete ImgDiff;
}

bool CompareArgs::Parse_Args(int argc, char **argv)
{
	if (argc < 3) {
		std::stringstream ss;
		ss << copyright;
		ss << usage;
		ss << "\n" << "OpenMP status: ";
#ifdef _OPENMP
		ss << "enabled\n";
#else
		ss << "disabled\n";
#endif
		ErrorStr = ss.str();
		return false;
	}
	unsigned int image_count = 0;
	const char *output_file_name = NULL;
	bool scale = false;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-fov") == 0) {
			if (++i < argc) {
				FieldOfView = atof(argv[i]);
			}
		} else if (strcmp(argv[i], "-verbose") == 0) {
			Verbose = true;
		} else if (strcmp(argv[i], "-threshold") == 0) {
			if (++i < argc) {
				ThresholdPixels = atoi(argv[i]);
			}
		} else if (strcmp(argv[i], "-gamma") == 0) {
			if (++i < argc) {
				Gamma = atof(argv[i]);
			}
		} else if (strcmp(argv[i], "-luminance") == 0) {
			if (++i < argc) {
				Luminance = atof(argv[i]);
			}
		} else if (strcmp(argv[i], "-luminanceonly") == 0) {
			LuminanceOnly = true;
		} else if (strcmp(argv[i], "-sum-errors") == 0) {
			SumErrors = true;
		} else if (strcmp(argv[i], "-colorfactor") == 0) {
			if (++i < argc) {
				ColorFactor = atof(argv[i]);
			}
		} else if (strcmp(argv[i], "-downsample") == 0) {
			if (++i < argc) {
				DownSample = atoi(argv[i]);
			}
		} else if (strcmp(argv[i], "-scale") == 0) {
			scale = true;
		} else if (strcmp(argv[i], "-output") == 0) {
			if (++i < argc) {
				output_file_name = argv[i];
			}
		} else if (image_count < 2) {
			RGBAImage *img = RGBAImage::ReadFromFile(argv[i]);
			if (!img) {
				ErrorStr = "FAIL: Cannot open ";
				ErrorStr += argv[i];
				ErrorStr += "\n";
				return false;
			} else {
				++image_count;
				if (image_count == 1) {
					ImgA = img;
				}
				else {
					ImgB = img;
				}
			}
		} else {
			fprintf(stderr, "Warning: option/file \"%s\" ignored\n", argv[i]);
		}
	} // i
	if (!ImgA || !ImgB) {
		ErrorStr = "FAIL: Not enough image files specified\n";
		return false;
	}
	for (unsigned int i = 0; i < DownSample; i++) {
		if (Verbose) {
			printf("Downsampling by %d\n", 1 << (i + 1));
		}
		RGBAImage *tmp = ImgA->DownSample();
		if (tmp) {
			delete ImgA;
			ImgA = tmp;
		}
		tmp = ImgB->DownSample();
		if (tmp) {
			delete ImgB;
			ImgB = tmp;
		}
	}
	if (scale &&
	        (ImgA->Get_Width() != ImgB->Get_Width() ||
	         ImgA->Get_Height() != ImgB->Get_Height())) {
		unsigned int min_width = ImgA->Get_Width();
		if (ImgB->Get_Width() < min_width) {
			min_width = ImgB->Get_Width();
		}

		unsigned int min_height = ImgA->Get_Height();
		if (ImgB->Get_Height() < min_height) {
			min_height = ImgB->Get_Height();
		}

		if (Verbose) {
			printf("Scaling to %u x %u\n", min_width, min_height);
		}
		RGBAImage *tmp = ImgA->DownSample(min_width, min_height);
		if (tmp) {
			delete ImgA;
			ImgA = tmp;
		}
		tmp = ImgB->DownSample(min_width, min_height);
		if (tmp) {
			delete ImgB;
			ImgB = tmp;
		}
	}
	if (output_file_name) {
		ImgDiff = new RGBAImage(ImgA->Get_Width(), ImgA->Get_Height(), output_file_name);
	}
	return true;
}

void CompareArgs::Print_Args() const
{
	printf("Field of view is %f degrees\n", FieldOfView);
	printf("Threshold pixels is %u pixels\n", ThresholdPixels);
	printf("The Gamma is %f\n", Gamma);
	printf("The Display's luminance is %f candela per meter squared\n", Luminance);
}
