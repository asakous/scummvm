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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/engines/saga/introproc_saga2.cpp $
 * $Id: introproc_saga2.cpp 49080 2010-05-18 14:57:02Z thebluegr $
 *
 */

#ifdef ENABLE_SAGA2

// "Dinotopia" and "Faery Tale Adventure II: Halls of the Dead" Intro sequence scene procedures

#include "saga/saga.h"
#include "saga/scene.h"
#include "saga/gfx.h"

#include "sound/mixer.h"
#include "graphics/surface.h"
#include "graphics/video/smk_decoder.h"

namespace Saga {

int Scene::DinoStartProc() {
	_vm->_gfx->showCursor(false);

	playMovie("testvid.smk");

	// HACK: Forcibly quit here
	_vm->quitGame();

	return SUCCESS;
}

int Scene::FTA2StartProc() {
	_vm->_gfx->showCursor(false);

	playMovie("trimark.smk");
	playMovie("intro.smk");

	// HACK: Forcibly quit here
	_vm->quitGame();

	return SUCCESS;
}

int Scene::FTA2EndProc(FTA2Endings whichEnding) {
	char videoName[20];

	switch (whichEnding) {
	case kFta2BadEndingLaw:
		strcpy(videoName, "end_1.smk");
		break;
	case kFta2BadEndingChaos:
		strcpy(videoName, "end_2.smk");
		break;
	case kFta2GoodEnding1:
		strcpy(videoName, "end_3a.smk");
		break;
	case kFta2GoodEnding2:
		strcpy(videoName, "end_3b.smk");
		break;
	case kFta2BadEndingDeath:
		strcpy(videoName, "end_4.smk");
		break;
	default:
		error("Unknown FTA2 ending");
	}

	_vm->_gfx->showCursor(false);

	// Play ending
	playMovie(videoName);

	return SUCCESS;
}

void Scene::playMovie(const char *filename) {
	Graphics::SmackerDecoder *smkDecoder = new Graphics::SmackerDecoder(_vm->_mixer);

	if (!smkDecoder->loadFile(filename))
		return;

	uint16 x = (g_system->getWidth() - smkDecoder->getWidth()) / 2;
	uint16 y = (g_system->getHeight() - smkDecoder->getHeight()) / 2;
	bool skipVideo = false;

	while (!_vm->shouldQuit() && !smkDecoder->endOfVideo() && !skipVideo) {
		if (smkDecoder->needsUpdate()) {
			Graphics::Surface *frame = smkDecoder->decodeNextFrame();
			if (frame) {
				_vm->_system->copyRectToScreen((byte *)frame->pixels, frame->pitch, x, y, frame->w, frame->h);

				if (smkDecoder->hasDirtyPalette())
					smkDecoder->setSystemPalette();

				_vm->_system->updateScreen();
			}
		}
	
		Common::Event event;
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) || event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}

		_vm->_system->delayMillis(10);
	}
}

} // End of namespace Saga

#endif
