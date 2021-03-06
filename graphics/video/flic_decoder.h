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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/graphics/video/flic_decoder.h $
 * $Id: flic_decoder.h 51725 2010-08-04 08:25:05Z fingolfin $
 *
 */

#ifndef GRAPHICS_VIDEO_FLICDECODER_H
#define GRAPHICS_VIDEO_FLICDECODER_H

#include "graphics/video/video_decoder.h"
#include "common/list.h"
#include "common/rect.h"

namespace Common {
	class SeekableReadStream;
}

namespace Graphics {

/**
 *
 * Decoder for FLIC videos.
 * Video decoder used in engines:
 *  - tucker
 */
class FlicDecoder : public FixedRateVideoDecoder {
public:
	FlicDecoder();
	virtual ~FlicDecoder();

	/**
	 * Load a video file
	 * @param stream  the stream to load
	 */
	bool load(Common::SeekableReadStream *stream);
	void close();

	/**
	 * Decode the next frame and return the frame's surface
	 * @note the return surface should *not* be freed
	 * @note this may return 0, in which case the last frame should be kept on screen
	 */
	Surface *decodeNextFrame();

	bool isVideoLoaded() const { return _fileStream != 0; }
	uint16 getWidth() const { return _surface->w; }
	uint16 getHeight() const { return _surface->h; }
	uint32 getFrameCount() const { return _frameCount; }
	PixelFormat getPixelFormat() const { return PixelFormat::createFormatCLUT8(); }

	const Common::List<Common::Rect> *getDirtyRects() const { return &_dirtyRects; }
	void clearDirtyRects() { _dirtyRects.clear(); }
	void copyDirtyRectsToBuffer(uint8 *dst, uint pitch);

	byte *getPalette() { _paletteChanged = false; return _palette; }
	bool hasDirtyPalette() const { return _paletteChanged; }
	void reset();

protected:
	Common::Rational getFrameRate() const { return _frameRate; }

private:
	uint16 _offsetFrame1;
	uint16 _offsetFrame2;
	byte *_palette;
	bool _paletteChanged;

	void decodeByteRun(uint8 *data);
	void decodeDeltaFLC(uint8 *data);
	void unpackPalette(uint8 *mem);

	Common::SeekableReadStream *_fileStream;
	Surface *_surface;
	uint32 _frameCount;
	Common::Rational _frameRate;

	Common::List<Common::Rect> _dirtyRects;
};

} // End of namespace Graphics

#endif
