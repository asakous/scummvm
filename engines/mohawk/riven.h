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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/engines/mohawk/riven.h $
 * $Id: riven.h 52661 2010-09-09 17:27:02Z mthreepwood $
 *
 */

#ifndef MOHAWK_RIVEN_H
#define MOHAWK_RIVEN_H

#include "mohawk/mohawk.h"
#include "mohawk/riven_scripts.h"

#include "gui/saveload.h"

#include "common/random.h"

namespace Mohawk {

struct MohawkGameDescription;
class MohawkArchive;
class RivenGraphics;
class RivenExternal;
class RivenConsole;
class RivenSaveLoad;
class RivenOptionsDialog;

#define RIVEN_STACKS 8

// Riven Stack Types
enum {
	aspit = 0,		// Main Menu, Books, Setup
	bspit = 1,		// Book-Making Island
	gspit = 2,		// Garden Island
	jspit = 3,		// Jungle Island
	ospit = 4,		// 233rd Age (Gehn's Office)
	pspit = 5,		// Prison Island
	rspit = 6,		// Rebel Age (Tay)
	tspit = 7		// Temple Island
};

// NAME Resource ID's
enum {
	CardNames = 1,
	HotspotNames = 2,
	ExternalCommandNames = 3,
	VariableNames = 4,
	StackNames = 5
};

// Rects for the inventory object positions (initialized in
// MohawkEngine_Riven's constructor).
extern Common::Rect *g_atrusJournalRect1;
extern Common::Rect *g_atrusJournalRect2;
extern Common::Rect *g_cathJournalRect2;
extern Common::Rect *g_atrusJournalRect3;
extern Common::Rect *g_cathJournalRect3;
extern Common::Rect *g_trapBookRect3;
extern Common::Rect *g_demoExitRect;

struct RivenHotspot {
	uint16 blstID;
	int16 name_resource;
	Common::Rect rect;
	uint16 u0;
	uint16 mouse_cursor;
	uint16 index;
	int16 u1;
	int16 zipModeHotspot;
	RivenScriptList scripts;

	bool enabled;
};

struct Card {
	int16 name;
	uint16 zipModePlace;
	bool hasData;
	RivenScriptList scripts;
};

struct ZipMode {
	Common::String name;
	uint16 id;
	bool operator== (const ZipMode& z) const;
};

class MohawkEngine_Riven : public MohawkEngine {
protected:
	Common::Error run();

public:
	MohawkEngine_Riven(OSystem *syst, const MohawkGameDescription *gamedesc);
	virtual ~MohawkEngine_Riven();

	RivenGraphics *_gfx;
	RivenExternal *_externalScriptHandler;
	Common::RandomSource *_rnd;
	RivenScriptManager *_scriptMan;

	Card _cardData;

	GUI::Debugger *getDebugger();

	bool canLoadGameStateCurrently() { return true; }
	bool canSaveGameStateCurrently() { return true; }
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const char *desc);
	bool hasFeature(EngineFeature f) const;

private:
	MohawkArchive *_extrasFile; // We need a separate handle for the extra data
	RivenConsole *_console;
	RivenSaveLoad *_saveLoad;
	RivenOptionsDialog *_optionsDialog;

	// Stack/Card-related functions and variables
	uint16 _curCard;
	uint16 _curStack;
	void loadCard(uint16);
	void handleEvents();

	// Hotspot related functions and variables
	uint16 _hotspotCount;
	void loadHotspots(uint16);
	void checkInventoryClick();
	bool _showHotspots;
	void updateZipMode();

	// Variables
	uint32 *_vars;
	uint32 _varCount;

	// Miscellaneous
	bool _gameOver;
	bool _ignoreNextMouseUp;

public:
	// Stack/card/script funtions
	void changeToCard(uint16 dest);
	void changeToStack(uint16);
	void refreshCard();
	Common::String getName(uint16 nameResource, uint16 nameID);
	Common::String getStackName(uint16 stack) const;
	void runCardScript(uint16 scriptType);
	void runUpdateScreenScript() { runCardScript(kCardUpdateScript); }
	uint16 getCurCard() const { return _curCard; }
	uint16 getCurStack() const { return _curStack; }
	uint16 matchRMAPToCard(uint32);
	uint32 getCurCardRMAP();

	// Hotspot functions/variables
	RivenHotspot *_hotspots;
	int32 _curHotspot;
	Common::Array<ZipMode> _zipModeData;
	uint16 getHotspotCount() const { return _hotspotCount; }
	void runHotspotScript(uint16 hotspot, uint16 scriptType);
	int32 getCurHotspot() const { return _curHotspot; }
	Common::String getHotspotName(uint16 hotspot);
	void checkHotspotChange();

	// Variable functions
	void initVars();
	uint32 getVarCount() const { return _varCount; }
	uint32 getGlobalVar(uint32 index);
	Common::String getGlobalVarName(uint32 index);
	uint32 *getLocalVar(uint32 index);
	uint32 *getVar(const Common::String &varName);

	// Miscellaneous
	void setGameOver() { _gameOver = true; }
	void ignoreNextMouseUp() { _ignoreNextMouseUp = true; }
	Common::SeekableReadStream *getExtrasResource(uint32 tag, uint16 id);
	bool _activatedSLST;
	void runLoadDialog();
	void delayAndUpdate(uint32 ms);
};

} // End of namespace Mohawk

#endif
