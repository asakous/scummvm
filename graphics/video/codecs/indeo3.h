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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/graphics/video/codecs/indeo3.h $
 * $Id: indeo3.h 52408 2010-08-27 10:45:19Z drmccoy $
 *
 */

#include "common/scummsys.h"

#ifdef USE_INDEO3

/* Intel Indeo 3 decompressor, derived from ffmpeg.
 *
 * Original copyright note:
 * Intel Indeo 3 (IV31, IV32, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#ifndef GRAPHICS_INDEO3_H
#define GRAPHICS_INDEO3_H

#include "graphics/video/codecs/codec.h"

namespace Graphics {

class Indeo3Decoder : public Codec {
public:
	Indeo3Decoder(uint16 width, uint16 height);
	~Indeo3Decoder();

	Surface *decodeImage(Common::SeekableReadStream *stream);
	PixelFormat getPixelFormat() const;

	static bool isIndeo3(Common::SeekableReadStream &stream);

private:
	Surface *_surface;

	PixelFormat _pixelFormat;

	static const int _corrector_type_0[24];
	static const int _corrector_type_2[8];
	static const uint32 correction[];
	static const uint32 correctionloworder[];
	static const uint32 correctionhighorder[];

	struct YUVBufs {
		byte *Ybuf;
		byte *Ubuf;
		byte *Vbuf;
		byte *the_buf;
		uint32 the_buf_size;
		uint16 y_w, y_h;
		uint16 uv_w, uv_h;
	};

	YUVBufs _iv_frame[2];
	YUVBufs *_cur_frame;
	YUVBufs *_ref_frame;

	byte *_ModPred;
	uint16 *_corrector_type;

	void buildModPred();
	void allocFrames();

	void decodeChunk(byte *cur, byte *ref, int width, int height,
			const byte *buf1, uint32 fflags2, const byte *hdr,
			const byte *buf2, int min_width_160);
};

} // End of namespace Graphics

#endif // GRAPHICS_INDEO3_H

#endif // USE_INDEO3
