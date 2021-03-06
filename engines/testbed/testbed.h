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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/engines/testbed/testbed.h $
 * $Id: testbed.h 51602 2010-08-01 23:26:38Z sud03r $
 */

#ifndef TESTBED_H
#define TESTBED_H

#include "engines/engine.h"

#include "gui/options.h"

#include "testbed/config.h"
#include "testbed/testsuite.h"

namespace Testbed {

class TestbedConfigManager;

enum {
	kTestbedLogOutput = 1 << 0,
	kTestbedEngineDebug = 1 << 2, 
	kCmdRerunTestbed = 'crtb'
};

class TestbedEngine : public Engine {
public:
	TestbedEngine(OSystem *syst);
	~TestbedEngine();

	virtual Common::Error run();

	/**
	 * Invokes configured testsuites.
	 */
	void invokeTestsuites(TestbedConfigManager &cfMan);

	bool hasFeature(EngineFeature f) const;
	
private:
	Common::Array<Testsuite *> _testsuiteList;
};

class TestbedExitDialog : public TestbedInteractionDialog {
public:
	TestbedExitDialog(Common::Array<Testsuite *> &testsuiteList) : TestbedInteractionDialog(80, 60, 500, 320), _rerun(false), 
	_testsuiteList(testsuiteList) {}
	~TestbedExitDialog() {}
	void init();
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	void run() { runModal(); }
	bool rerunRequired() {
		if (_rerun) {
			_rerun = false;
			return true;
		}
		return false;
	}
private:
	bool _rerun;
	Common::Array<Testsuite *> &_testsuiteList;
};

} // End of namespace Testbed

#endif // TESTBED_H
