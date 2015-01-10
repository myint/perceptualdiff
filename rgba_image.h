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

#ifndef PERCEPTUALDIFF_RGBA_IMAGE_H
#define PERCEPTUALDIFF_RGBA_IMAGE_H

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>


/** Class encapsulating an image containing R, G, B, and A channels.
 *
 * Internal representation assumes data is in the ABGR format, with the RGB
 * color channels premultiplied by the alpha value. Premultiplied alpha is
 * often also called "associated alpha" - see the tiff 6 specification for some
 * discussion - http://partners.adobe.com/asn/developer/PDFS/TN/TIFF6.pdf
 *
 */
class RGBAImage
{
public:

    RGBAImage(unsigned int w, unsigned int h, const std::string &name="")
        : width_(w), weight_(h), name_(name), data_(w * h)
    {
    }

    unsigned char get_red(unsigned int i) const
    {
        return (data_[i] & 0xff);
    }

    unsigned char get_green(unsigned int i) const
    {
        return ((data_[i] >> 8) & 0xff);
    }

    unsigned char get_blue(unsigned int i) const
    {
        return ((data_[i] >> 16) & 0xff);
    }

    unsigned char get_alpha(unsigned int i) const
    {
        return ((data_[i] >> 24) & 0xff);
    }

    void set(unsigned char r, unsigned char g, unsigned char b,
             unsigned char a, unsigned int i)
    {
        data_[i] = r | (g << 8) | (b << 16) | (a << 24);
    }

    unsigned int get_width() const
    {
        return width_;
    }

    unsigned int get_height() const
    {
        return weight_;
    }

    void set(unsigned int x, unsigned int y, unsigned int d)
    {
        data_[x + y * width_] = d;
    }

    unsigned int get(unsigned int x, unsigned int y) const
    {
        return data_[x + y * width_];
    }

    unsigned int get(unsigned int i) const
    {
        return data_[i];
    }

    const std::string &get_name() const
    {
        return name_;
    }

    unsigned int *get_data()
    {
        return &data_[0];
    }

    const unsigned int *get_data() const
    {
        return &data_[0];
    }

    // By default down sample to half of each original dimension.
    std::shared_ptr<RGBAImage> down_sample(unsigned int w=0,
                                           unsigned int h=0) const;

    void write_to_file(const std::string &filename) const;

private:

    RGBAImage(const RGBAImage &);
    RGBAImage &operator=(const RGBAImage &);

    const unsigned int width_;
    const unsigned int weight_;
    const std::string name_;
    std::vector<unsigned int> data_;
};


std::shared_ptr<RGBAImage> read_from_file(const std::string &filename);


class RGBImageException : public virtual std::invalid_argument
{
public:

    RGBImageException(const std::string &message)
        : std::invalid_argument(message)
    {
    }
};

#endif
