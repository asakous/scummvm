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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/graphics/video/codecs/cinepak.h $
 * $Id: cinepak.h 49165 2010-05-23 18:33:55Z mthreepwood $
 *
 */

#ifndef GRAPHICS_CINEPAK_H
#define GRAPHICS_CINEPAK_H

#include "common/scummsys.h"
#include "common/stream.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "graphics/pixelformat.h"

#include "graphics/video/codecs/codec.h"

namespace Graphics {

struct CinepakCodebook {
	byte y[4];
	byte u, v;
};

struct CinepakStrip {
	uint16 id;
	uint16 length;
	Common::Rect rect;
	CinepakCodebook v1_codebook[256], v4_codebook[256];
};

struct CinepakFrame {
	byte flags;
	uint32 length;
	uint16 width;
	uint16 height;
	uint16 stripCount;
	CinepakStrip *strips;

	Surface *surface;
};

class CinepakDecoder : public Codec {
public:
	CinepakDecoder();
	~CinepakDecoder();

	Surface *decodeImage(Common::SeekableReadStream *stream);
	PixelFormat getPixelFormat() const { return _pixelFormat; }

private:
	CinepakFrame _curFrame;
	int32 _y;
	PixelFormat _pixelFormat;

	void loadCodebook(Common::SeekableReadStream *stream, uint16 strip, byte codebookType, byte chunkID, uint32 chunkSize);
	void decodeVectors(Common::SeekableReadStream *stream, uint16 strip, byte chunkID, uint32 chunkSize);
};

} // End of namespace Graphics

#endif
