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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/engines/drascula/console.h $
 * $Id: console.h 49870 2010-06-15 17:14:38Z wjpalenstijn $
 *
 */

#ifndef DRASCULA_CONSOLE_H
#define DRASCULA_CONSOLE_H

#include "gui/debugger.h"

namespace Drascula {

class DrasculaEngine;

class Console : public GUI::Debugger {
public:
	Console(DrasculaEngine *vm);
	virtual ~Console(void);

protected:
	virtual void preEnter();
	virtual void postEnter();

private:
	DrasculaEngine *_vm;

	bool Cmd_Room(int argc, const char **argv);
};

} // End of namespace Drascula
#endif
