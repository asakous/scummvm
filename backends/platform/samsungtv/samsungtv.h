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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/backends/platform/samsungtv/samsungtv.h $
 * $Id: samsungtv.h 48148 2010-02-27 17:02:58Z jvprat $
 *
 */

#ifndef SDL_SAMSUNGTV_COMMON_H
#define SDL_SAMSUNGTV_COMMON_H

#include <SDL.h>

#include "backends/base-backend.h"
#include "backends/platform/sdl/sdl.h"

#if defined(SAMSUNGTV)

namespace Audio {
	class MixerImpl;
}

class OSystem_SDL_SamsungTV : public OSystem_SDL {
public:
	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);

protected:

	virtual bool remapKey(SDL_Event &ev, Common::Event &event);
};

#endif

#endif
