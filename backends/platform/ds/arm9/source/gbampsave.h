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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/backends/platform/ds/arm9/source/gbampsave.h $
 * $Id: gbampsave.h 50741 2010-07-07 23:22:53Z fingolfin $
 *
 */

#ifndef _GBAMPSAVE_H_
#define _GBAMPSAVE_H_

#include "common/system.h"
#include "common/savefile.h"
#include "backends/fs/ds/ds-fs.h"

#define SAVE_BUFFER_SIZE 100000

class GBAMPSaveFile : public Common::InSaveFile, public Common::OutSaveFile {
	DS::fileHandle *handle;
	char buffer[SAVE_BUFFER_SIZE];
	int bufferPos;
	int saveSize;
	int flushed;

public:
	GBAMPSaveFile(char *name, bool saveOrLoad);
	virtual ~GBAMPSaveFile();

	virtual uint32 read(void *buf, uint32 size);
	virtual uint32 write(const void *buf, uint32 size);

	virtual bool eos() const;
	virtual bool skip(uint32 bytes);

	virtual int32 pos() const;
	virtual int32 size() const;
	virtual bool seek(int32 pos, int whence);

	void flushSaveBuffer();

	virtual bool isOpen() const {
		return handle != 0;
	}
};


class GBAMPSaveFileManager : public Common::SaveFileManager {
public:
	GBAMPSaveFileManager();
	~GBAMPSaveFileManager();

//	static GBAMPSaveFileManager *instance() { return instancePtr; }

	GBAMPSaveFile *openSavefile(const char *filename, bool saveOrLoad);

	virtual Common::OutSaveFile *openForSaving(const Common::String &filename) { return openSavefile(filename.c_str(), true); }
	virtual Common::InSaveFile *openForLoading(const Common::String &filename) { return openSavefile(filename.c_str(), false); }

	virtual bool removeSavefile(const Common::String &filename) { return false; } // TODO: Implement this
	virtual Common::StringArray listSavefiles(const Common::String &pattern);

	void deleteFile(const Common::String &name);
	void listFiles();

	const char *getSavePath() const;
};

#endif
