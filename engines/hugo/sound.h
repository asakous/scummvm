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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/engines/hugo/sound.h $
 * $Id: sound.h 52406 2010-08-27 09:48:53Z strangerke $
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_SOUND_H
#define HUGO_SOUND_H

#include "sound/mixer.h"

namespace Hugo {

class MidiPlayer;
	
class SoundHandler {
public:
	SoundHandler(HugoEngine &vm);

	void toggleMusic();
	void toggleSound();
	void setMusicVolume();
	void playMusic(short tune);
	void playSound(short sound, stereo_t channel, byte priority);
	void initSound();

private:
	HugoEngine &_vm;
	Audio::SoundHandle _soundHandle;
	MidiPlayer *_midiPlayer;

	void stopSound();
	void stopMusic();
	void playMIDI(sound_pt seq_p, uint16 size);
};

} // End of namespace Hugo

#endif //HUGO_SOUND_H
