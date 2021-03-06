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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/engines/sci/engine/savegame.h $
 * $Id: savegame.h 52876 2010-09-24 10:16:16Z wjpalenstijn $
 *
 */

#ifndef SCI_ENGINE_SAVEGAME_H
#define SCI_ENGINE_SAVEGAME_H

#include "common/scummsys.h"
#include "common/str.h"

#include "sci/sci.h"

namespace Sci {

struct EngineState;

enum {
	CURRENT_SAVEGAME_VERSION = 25,
	MINIMUM_SAVEGAME_VERSION = 14
};

// Savegame metadata
struct SavegameMetadata {
	Common::String savegame_name;
	int savegame_version;
	Common::String game_version;
	int savegame_date;
	int savegame_time;
	uint16 game_object_offset;
	uint16 script0_size;
};


/**
 * Saves a game state to the hard disk in a portable way.
 * @param s			The state to save
 * @param save		The stream to save to
 * @param savename	The description of the savegame
 * @return 0 on success, 1 otherwise
 */
bool gamestate_save(EngineState *s, Common::WriteStream *save, const char *savename, const char *version);

/**
 * Restores a game state from a directory.
 * @param s			An older state from the same game
 * @param dirname	The subdirectory to restore from
 */
void gamestate_restore(EngineState *s, Common::SeekableReadStream *save);

/**
 * Read the header from a savegame.
 */
bool get_savegame_metadata(Common::SeekableReadStream* stream, SavegameMetadata* meta);


} // End of namespace Sci

#endif // SCI_ENGINE_SAVEGAME_H
