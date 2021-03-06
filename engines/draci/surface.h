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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/engines/draci/surface.h $
 * $Id: surface.h 48936 2010-05-04 12:00:16Z fingolfin $
 *
 */

#ifndef DRACI_SURFACE_H
#define DRACI_SURFACE_H

#include "common/list.h"
#include "graphics/surface.h"

namespace Draci {

class Surface : public Graphics::Surface {

public:
	Surface(int width, int height);
	~Surface();

	void markDirtyRect(Common::Rect r);
	const Common::List<Common::Rect> *getDirtyRects() const { return &_dirtyRects; }
	void clearDirtyRects() { _dirtyRects.clear(); }
	void markDirty();
	void markClean();
	bool needsFullUpdate() const { return _fullUpdate; }
	uint getTransparentColour() const { return _transparentColour; }
	void setTransparentColour(uint colour) { _transparentColour = colour; }
	void fill(uint colour);
	uint putAboveY(int y, int height) const;
	uint centerOnX(int x, int width) const;
	Common::Rect getDimensions() const { return Common::Rect(w, h); }

private:
	/** The current transparent colour of the surface. See getTransparentColour() and
	 *  setTransparentColour().
	 */
	uint _transparentColour;

	/** Set when the surface is scheduled for a full update.
	 *  See markDirty(), markClean(). Accessed via needsFullUpdate().
	 */
	bool _fullUpdate;

	Common::List<Common::Rect> _dirtyRects; ///< List of currently dirty rectangles

};

} // End of namespace Draci

#endif // DRACI_SURFACE_H
