/*
Metric
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

#include "Metric.h"
#include "CompareArgs.h"
#include "RGBAImage.h"
#include "LPyramid.h"
#include <math.h>

/*
* Given the adaptation luminance, this function returns the
* threshold of visibility in cd per m^2
* TVI means Threshold vs Intensity function
* This version comes from Ward Larson Siggraph 1997
*/ 

float TVI(float adaptation_luminance)
{
      // returns the threshold luminance given the adaptation luminance
      // units are candelas per meter squared

      float log_a, r, result; 
      log_a = log10(adaptation_luminance);

      if (log_a < -3.94) {
            r = -2.86;
      } else if (log_a < -1.44) {
            r = powf(0.405 * log_a + 1.6 , 2.18) - 2.86;
      } else if (log_a < -0.0184) {
            r = log_a - 0.395;
      } else if (log_a < 1.9) {
            r = powf(0.249 * log_a + 0.65, 2.7) - 0.72;
      } else {
            r = log_a - 1.255;
      }

      result = powf(10.0 , r); 

      return result;

} 

// computes the contrast sensitivity function (Barten SPIE 1989)
// given the cycles per degree (cpd) and luminance (lum)
float csf(float cpd, float lum)
{
	float a, b, result; 
	
	a = 440 * powf((1 + 0.7 / lum), -0.2);
	b = 0.3 * powf((1 + 100.0 / lum), 0.15);
		
	result = a * cpd * expf(-b * cpd) * sqrtf(1 + 0.06 * expf(b * cpd)); 
	
	return result;	
}

/*
* Visual Masking Function
* from Daly 1993
*/
float mask(float contrast)
{
      float a, b, result;
      a = pow(392.498 * contrast,  0.7);
      b = pow(0.0153 * a, 4);
      result = pow (1 + b, 0.25); 

      return result;
} 

// convert Adobe RGB (1998) with reference white D65 to XYZ
void AdobeRGBToXYZ(float r, float g, float b, float &x, float &y, float &z)
{
	// matrix is from http://www.brucelindbloom.com/
	x = r * 0.576700f + g * 0.185556f + b * 0.188212f;
	y = r * 0.297361f + g * 0.627355f + b * 0.0752847f;
	z = r * 0.0270328f + g * 0.0706879f + b * 0.991248f;
}

bool Yee_Compare(CompareArgs &args)
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
	
	if (args.Verbose) printf("Constructing Laplacian Pyramids\n");
	
	LPyramid *la = new LPyramid(aLum, w, h);
	LPyramid *lb = new LPyramid(bLum, w, h);
	
	if (aX) delete[] aX;
	if (aY) delete[] aY;
	if (aZ) delete[] aZ;
	if (bX) delete[] bX;
	if (bY) delete[] bY;
	if (bZ) delete[] bZ;
	if (aLum) delete[] aLum;
	if (bLum) delete[] bLum;
	if (la) delete la;
	if (lb) delete lb;

	args.ErrorStr = "Images are visibly different\n";
	return false;
}