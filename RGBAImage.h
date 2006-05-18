/*
RGBAImage.h
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
	
	static RGBAImage* ReadTiff(char *filename);
protected:
	int Width;
	int Height;
	unsigned int *Data;
};