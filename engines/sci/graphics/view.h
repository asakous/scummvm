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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/engines/sci/graphics/view.h $
 * $Id: view.h 53863 2010-10-26 21:59:36Z thebluegr $
 *
 */

#ifndef SCI_GRAPHICS_VIEW_H
#define SCI_GRAPHICS_VIEW_H

namespace Sci {

struct CelInfo {
	int16 width, height;
	int16 scriptWidth, scriptHeight;
	int16 displaceX;
	int16 displaceY;
	byte clearKey;
	uint16 offsetEGA;
	uint32 offsetRLE;
	uint32 offsetLiteral;
	byte *rawBitmap;
};

struct LoopInfo {
	bool mirrorFlag;
	uint16 celCount;
	CelInfo *cel;
};

#define SCI_VIEW_EGAMAPPING_SIZE 16
#define SCI_VIEW_EGAMAPPING_COUNT 8

class GfxScreen;
class GfxPalette;

/**
 * View class, handles loading of view resources and drawing contained cels to screen
 *  every view resource has its own instance of this class
 */
class GfxView {
public:
	GfxView(ResourceManager *resMan, GfxScreen *screen, GfxPalette *palette, GuiResourceId resourceId);
	~GfxView();

	GuiResourceId getResourceId() const;
	int16 getWidth(int16 loopNo, int16 celNo) const;
	int16 getHeight(int16 loopNo, int16 celNo) const;
	const CelInfo *getCelInfo(int16 loopNo, int16 celNo) const;
	void getCelRect(int16 loopNo, int16 celNo, int16 x, int16 y, int16 z, Common::Rect &outRect) const;
	void getCelSpecialHoyle4Rect(int16 loopNo, int16 celNo, int16 x, int16 y, int16 z, Common::Rect &outRect) const;
	void getCelScaledRect(int16 loopNo, int16 celNo, int16 x, int16 y, int16 z, int16 scaleX, int16 scaleY, Common::Rect &outRect) const;
	const byte *getBitmap(int16 loopNo, int16 celNo);
	void draw(const Common::Rect &rect, const Common::Rect &clipRect, const Common::Rect &clipRectTranslated, int16 loopNo, int16 celNo, byte priority, uint16 EGAmappingNr, bool upscaledHires);
	void drawScaled(const Common::Rect &rect, const Common::Rect &clipRect, const Common::Rect &clipRectTranslated, int16 loopNo, int16 celNo, byte priority, int16 scaleX, int16 scaleY);
	uint16 getLoopCount() const { return _loopCount; }
	uint16 getCelCount(int16 loopNo) const;
	Palette *getPalette();

	bool isScaleable();
	bool isSci2Hires();

private:
	void initData(GuiResourceId resourceId);
	void unpackCel(int16 loopNo, int16 celNo, byte *outPtr, uint32 pixelCount);
	void unditherBitmap(byte *bitmap, int16 width, int16 height, byte clearKey);

	ResourceManager *_resMan;
	GfxCoordAdjuster *_coordAdjuster;
	GfxScreen *_screen;
	GfxPalette *_palette;

	GuiResourceId _resourceId;
	Resource *_resource;
	byte *_resourceData;
	int _resourceSize;

	uint16 _loopCount;
	LoopInfo *_loop;
	bool _embeddedPal;
	Palette _viewPalette;

	// set for SCI2 views in gk1/windows, means that views are hires and should be handled accordingly
	bool _isSci2Hires;

	byte *_EGAmapping;

	// this is set for sci0early to adjust for the getCelRect() change
	int16 _adjustForSci0Early;

	// this is not set for some views in laura bow 2 floppy and signals that the view shall never get scaled
	//  even if scaleX/Y are set (inside kAnimate)
	bool _isScaleable;
};

} // End of namespace Sci

#endif
