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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/backends/platform/ps2/savefilemgr.h $
 * $Id: savefilemgr.h 48286 2010-03-18 15:44:59Z fingolfin $
 *
 */

#ifndef __SAVEFILEMGR_H__
#define __SAVEFILEMGR_H__

// #include "common/savefile.h"
#include "backends/saves/default/default-saves.h"

class Gs2dScreen;
class OSystem_PS2;

class Ps2SaveFileManager : public Common::SaveFileManager {
public:
	Ps2SaveFileManager(OSystem_PS2 *system, Gs2dScreen *screen);
	virtual ~Ps2SaveFileManager();

	virtual Common::InSaveFile *openForLoading(const Common::String &filename);
	virtual Common::OutSaveFile *openForSaving(const Common::String &filename);
	virtual Common::StringArray listSavefiles(const Common::String &pattern);
	virtual bool removeSavefile(const Common::String &filename);

	// void writeSaveNonblocking(char *name, void *buf, uint32 size);
	// void saveThread(void);
	// void quit(void);

private:
	bool mcCheck(const char *dir);
	void mcSplit(char *full, char *game, char *ext);

	int _sema;
	Gs2dScreen *_screen;
};

#endif // __SAVEFILE_MGR_H__
