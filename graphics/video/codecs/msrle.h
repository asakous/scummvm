/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/graphics/video/codecs/msrle.h $
 * $Id: msrle.h 49079 2010-05-18 14:17:24Z mthreepwood $
 *
 */

#ifndef GRAPHICS_MSRLE_H
#define GRAPHICS_MSRLE_H

#include "graphics/video/codecs/codec.h"

namespace Graphics {

class MSRLEDecoder : public Codec {
public:
	MSRLEDecoder(uint16 width, uint16 height, byte bitsPerPixel);
	~MSRLEDecoder();

	Surface *decodeImage(Common::SeekableReadStream *stream);
	PixelFormat getPixelFormat() const { return PixelFormat::createFormatCLUT8(); }

private:
	byte _bitsPerPixel;

	Surface *_surface;

	void decode8(Common::SeekableReadStream *stream);
};

} // End of namespace Graphics

#endif
