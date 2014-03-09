/*
RGBAImage.h
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

#ifndef _RGBAIMAGE_H
#define _RGBAIMAGE_H

#include <memory>
#include <string>
#include <vector>

/** Class encapsulating an image containing R,G,B,A channels.
 *
 * Internal representation assumes data is in the ABGR format, with the RGB
 * color channels premultiplied by the alpha value. Premultiplied alpha is
 * often also called "associated alpha" - see the tiff 6 specification for some
 * discussion - http://partners.adobe.com/asn/developer/PDFS/TN/TIFF6.pdf
 *
 */
class RGBAImage
{
    RGBAImage(const RGBAImage &);
    RGBAImage &operator=(const RGBAImage &);

public:
    RGBAImage(unsigned int w, unsigned int h, const std::string &name="")
        : Width(w), Height(h), Name(name), Data(w * h)
    {
    }
    unsigned char Get_Red(unsigned int i) const
    {
        return (Data[i] & 0xFF);
    }
    unsigned char Get_Green(unsigned int i) const
    {
        return ((Data[i] >> 8) & 0xFF);
    }
    unsigned char Get_Blue(unsigned int i) const
    {
        return ((Data[i] >> 16) & 0xFF);
    }
    unsigned char Get_Alpha(unsigned int i) const
    {
        return ((Data[i] >> 24) & 0xFF);
    }
    void Set(unsigned char r, unsigned char g, unsigned char b,
             unsigned char a, unsigned int i)
    {
        Data[i] = r | (g << 8) | (b << 16) | (a << 24);
    }
    unsigned int Get_Width(void) const
    {
        return Width;
    }
    unsigned int Get_Height(void) const
    {
        return Height;
    }
    void Set(unsigned int x, unsigned int y, unsigned int d)
    {
        Data[x + y * Width] = d;
    }
    unsigned int Get(unsigned int x, unsigned int y) const
    {
        return Data[x + y * Width];
    }
    unsigned int Get(unsigned int i) const
    {
        return Data[i];
    }
    const std::string &Get_Name(void) const
    {
        return Name;
    }
    unsigned int *Get_Data()
    {
        return &Data[0];
    }
    const unsigned int *Get_Data() const
    {
        return &Data[0];
    }

    /** By default down sample to half of each original dimension.
     */
    std::shared_ptr<RGBAImage> DownSample(unsigned int w=0,
                                          unsigned int h=0) const;

    bool WriteToFile(const std::string &filename) const;
    static std::shared_ptr<RGBAImage>
    ReadFromFile(const std::string &filename);

private:
    const unsigned int Width;
    const unsigned int Height;
    const std::string Name;
    std::vector<unsigned int> Data;
};

#endif
