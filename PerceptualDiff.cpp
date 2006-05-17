/*
PerceptualDiff - a program that compares two images using a perceptual metric
based on the paper :
A perceptual metric for production testing. Journal of graphics tools, 9(4):33-40, 2004, Hector Yee
Copyright (C) 2006 Yangli Hector Yee

This program is free software; you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program;
if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string>
#include "LPyramid.h"
#include "tiff.h"
#include "tiffio.h"

const char* copyright = 
"PerceptualDiff version 0.1, Copyright (C) 2006 Yangli Hector Yee\n\
PerceptualDiff comes with ABSOLUTELY NO WARRANTY;\n\
This is free software, and you are welcome\n\
to redistribute it under certain conditions;\n\
See the GPL page for details: http://www.gnu.org/copyleft/gpl.html\n\n";

const char *usage =
"PeceptualDiff image1.tif image2.tif\n\n\
   Compares image1.tif and image2.tif using a perceptually based image metric\n\
   Options:\n\
\t-verbose       : Turns on verbose mode\n\
\t-fov deg       : Field of view in degrees (0.1 to 89.9)\n\
\t-threshold p	 : #pixels p below which differences are ignored\n\
\t-gamma g       : Value to convert rgb into linear space (default 2.2)\n\
\t-luminance l   : White luminance (default 100.0 cdm^-2)\n\
\n";

// assumes data is in the ABGR format
class RGBAImage
{
public:
	RGBAImage(int w, int h)
	{
		Width = w;
		Height = h;
		Data = new unsigned int[w * h];
	};
	~RGBAImage() { if (Data) delete[] Data; }
	unsigned char Get_Red(unsigned int i) { return (Data[i] & 0xFF); }
	unsigned char Get_Green(unsigned int i) { return ((Data[i]>>8) & 0xFF); }
	unsigned char Get_Blue(unsigned int i) { return ((Data[i]>>16) & 0xFF); }
	unsigned char Get_Alpha(unsigned int i) { return ((Data[i]>>24) & 0xFF); }
	int Get_Width(void) const { return Width; }
	int Get_Height(void) const { return Height; }
	void Set(int x, int y, unsigned int d) { Data[x + y * Width] = d; }
	unsigned int Get(int x, int y) const { return Data[x + y * Width]; }
	unsigned int Get(int i) const { return Data[i]; }
protected:
	int Width;
	int Height;
	unsigned int *Data;
};

RGBAImage* ReadImage(char *filename);

// Args to pass into the comparison function
struct Compare_Args
{
	Compare_Args()
	{
		ImgA = NULL;
		ImgB = NULL;
		Verbose = false;
		FieldOfView = 45.0f;
		Gamma = 2.2f;
		ThresholdPixels = 100;
		Luminance = 100.0f;
	}
	bool Parse_Args(int argc, char **argv)
	{
		if (argc < 3) {
			ErrorStr = copyright;
			ErrorStr += usage;
			return false;
		}
		for (int i = 0; i < argc; i++) {
			if (i == 1) {
				ImgA = ReadImage(argv[1]);
				if (!ImgA) {
					ErrorStr = "FAIL: Cannot open ";
					ErrorStr += argv[1];
					ErrorStr += "\n";
					return false;
				}
			} else if (i == 2) {			
				ImgB = ReadImage(argv[2]);
				if (!ImgB) {
					ErrorStr = "FAIL: Cannot open ";
					ErrorStr += argv[2];
					ErrorStr += "\n";
					return false;
				}
			} else {
				if (strstr(argv[i], "-fov")) {
					if (i + 1 < argc) {
						FieldOfView = atof(argv[i + 1]);
					}
				} else if (strstr(argv[i], "-verbose")) {
					Verbose = true;
				} else 	if (strstr(argv[i], "-threshold")) {
					if (i + 1 < argc) {
						ThresholdPixels = atoi(argv[i + 1]);
					}
				} else 	if (strstr(argv[i], "-gamma")) {
					if (i + 1 < argc) {
						Gamma = atof(argv[i + 1]);
					}
				}else 	if (strstr(argv[i], "-luminance")) {
					if (i + 1 < argc) {
						Luminance = atof(argv[i + 1]);
					}
				}
			}
		} // i
		return true;
	}
	void Print_Args()
	{
		printf("Field of view is %f degrees\n", FieldOfView);
		printf("Threshold pixels is %d pixels\n", ThresholdPixels);
		printf("The Gamma is %f\n", Gamma);
		printf("The Display's luminance is %f candela per meter squared\n", Luminance);
	}
	~Compare_Args()
	{
		if (ImgA) delete ImgA;
		if (ImgB) delete ImgB;
	}
	RGBAImage		*ImgA;				// Image A
	RGBAImage		*ImgB;				// Image B
	bool			Verbose;			// Print lots of text or not
	float			FieldOfView;		// Field of view in degrees
	float			Gamma;				// The gamma to convert to linear color space
	float			Luminance;			// the display's luminance
	unsigned int	ThresholdPixels;	// How many pixels different to ignore
	std::string		ErrorStr;			// Error string
};

RGBAImage* ReadImage(char *filename)
{
	RGBAImage *fimg = 0;
	
    TIFF* tif = TIFFOpen(filename, "r");
    char emsg[1024];
    if (tif) {
		TIFFRGBAImage img;
		
		if (TIFFRGBAImageBegin(&img, tif, 0, emsg)) {
			size_t npixels;
			uint32* raster;

			npixels = img.width * img.height;
			raster = (uint32*) _TIFFmalloc(npixels * sizeof (uint32));
			if (raster != NULL) {
				if (TIFFRGBAImageGet(&img, raster, img.width, img.height)) {
					// result is in ABGR
					fimg = new RGBAImage(img.width, img.height);
					for (int y = 0; y < img.height; y++) {
						for (int x = 0; x < img.width; x++) {
						   fimg->Set(x,y, raster[x + y * img.width]);
						}
					}
				}
			_TIFFfree(raster);
			}
	    }
	    TIFFRGBAImageEnd(&img);
	} else {
	    TIFFError(filename, emsg);
	}
	return fimg;
}

float tviWard97(float luminance)
// combines both photopic and scotopic TVI functions
// in Ward and Rushmeier 97
{
	if (luminance< 1.14815e-4) return (float) 1.38038426e-3;

	double loglum=log10(luminance);
	
	if (loglum<-1.44) return (float) (pow(10.0, pow(0.405*loglum+1.6,2.18) - 2.86) );
	if (loglum<-0.0184) return (float) (pow(10.0,loglum-0.395));
	if (loglum<1.9) return (float) (pow(10.0, pow(0.249*loglum + 0.65,2.7) - 0.72) );
	return luminance*5.559e-2f;
}


// convert Adobe RGB (1998) with reference white D65 to XYZ
void AdobeRGBToXYZ(float r, float g, float b, float &x, float &y, float &z)
{
	// matrix is from http://www.brucelindbloom.com/
	x = r * 0.576700f + g * 0.185556f + b * 0.188212f;
	y = r * 0.297361f + g * 0.627355f + b * 0.0752847f;
	z = r * 0.0270328f + g * 0.0706879f + b * 0.991248f;
}

bool Compare_Images(Compare_Args &args)
{
	if ((args.ImgA->Get_Width() != args.ImgB->Get_Width()) ||
		(args.ImgA->Get_Height() != args.ImgB->Get_Height())) {
		args.ErrorStr = "Image dimensions do not match\n";
		return false;
	}
	
	unsigned int i, dim;
	dim = args.ImgA->Get_Width() * args.ImgA->Get_Height();
	bool identical = true;
	for (i = 0; i < dim; i++) {
		if (args.ImgA->Get(i) != args.ImgB->Get(i)) {
		  identical = false;
		  break;
		}
	}
	if (identical) {
		args.ErrorStr = "Images are binary identical\n";
		return true;
	}
	
	// assuming colorspaces are in Adobe RGB (1998) convert to XYZ
	float *aX = new float[dim];
	float *aY = new float[dim];
	float *aZ = new float[dim];
	float *bX = new float[dim];
	float *bY = new float[dim];
	float *bZ = new float[dim];
	float *aLum = new float[dim];
	float *bLum = new float[dim];
	
	if (args.Verbose) printf("Converting RGB to XYZ\n");
	
	unsigned int x, y, w, h;
	w = args.ImgA->Get_Width();
	h = args.ImgA->Get_Height();
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			float r, g, b;
			i = x + y * w;
			r = powf(args.ImgA->Get_Red(i) / 255.0f, args.Gamma);
			g = powf(args.ImgA->Get_Green(i) / 255.0f, args.Gamma);
			b = powf(args.ImgA->Get_Blue(i) / 255.0f, args.Gamma);						
			AdobeRGBToXYZ(r,g,b,aX[i],aY[i],aZ[i]);
			r = powf(args.ImgB->Get_Red(i) / 255.0f, args.Gamma);
			g = powf(args.ImgB->Get_Green(i) / 255.0f, args.Gamma);
			b = powf(args.ImgB->Get_Blue(i) / 255.0f, args.Gamma);						
			AdobeRGBToXYZ(r,g,b,bX[i],bY[i],bZ[i]);
			aLum[i] = aY[i] * args.Luminance;
			bLum[i] = bY[i] * args.Luminance;
		}
	}
	
	if (aX) delete[] aX;
	if (aY) delete[] aY;
	if (aZ) delete[] aZ;
	if (bX) delete[] bX;
	if (bY) delete[] bY;
	if (bZ) delete[] bZ;
	if (aLum) delete[] aLum;
	if (bLum) delete[] bLum;

	args.ErrorStr = "Images are visibly different\n";
	return false;
}

int main(int argc, char **argv)
{
	Compare_Args args;
	
	if (!args.Parse_Args(argc, argv)) {
		printf("%s", args.ErrorStr.c_str());
		return -1;
	} else {
		if (args.Verbose) args.Print_Args();
	}
	int result = Compare_Images(args) == true;
	if (result) {
		printf("PASS: %s\n", args.ErrorStr.c_str());
	} else {
		printf("FAIL: %s\n", args.ErrorStr.c_str());
	}
	return result;
}
