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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/backends/saves/savefile.cpp $
 * $Id: savefile.cpp 41000 2009-05-29 14:38:22Z fingolfin $
 *
 */

#include "common/util.h"
#include "common/savefile.h"

#include <stdio.h>
#include <string.h>

namespace Common {

bool SaveFileManager::renameSavefile(const String &oldFilename, const String &newFilename) {

	InSaveFile *inFile = 0;
	OutSaveFile *outFile = 0;
	uint32 size = 0;
	void *buffer = 0;
	bool success = false;

	inFile = openForLoading(oldFilename);

	if (inFile) {
		size = inFile->size();
		buffer = malloc(size);
		assert(buffer);

		outFile = openForSaving(newFilename);

		if (buffer && outFile) {
			inFile->read(buffer, size);
			bool error = inFile->err();
			delete inFile;
			inFile = 0;

			if (!error) {
				outFile->write(buffer, size);
				outFile->finalize();
				if (!outFile->err()) {
					success = removeSavefile(oldFilename);
				}
			}
		}

		free(buffer);
		delete outFile;
		delete inFile;
	}

	return success;
}

String SaveFileManager::popErrorDesc() {
	String err = _errorDesc;
	clearError();

	return err;
}

} // End of namespace Common
