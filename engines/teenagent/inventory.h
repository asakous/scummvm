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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/engines/teenagent/inventory.h $
 * $Id: inventory.h 48350 2010-03-21 07:49:53Z megath $
 */

#ifndef TEENAGENT_INVENTORY_H
#define TEENAGENT_INVENTORY_H

#include "teenagent/surface.h"
#include "teenagent/animation.h"
#include "common/events.h"
#include "common/array.h"
#include "teenagent/objects.h"

namespace TeenAgent {

struct InventoryObject;
class TeenAgentEngine;

class Inventory {
public:
	void init(TeenAgentEngine *engine);
	void render(Graphics::Surface *surface, int delta);

	void clear();
	void reload();
	void add(byte item);
	bool has(byte item) const;
	void remove(byte item);

	void activate(bool a) { _active = a; }
	bool active() const { return _active; }

	bool processEvent(const Common::Event &event);

	InventoryObject *selectedObject() { return selected_obj; }
	void resetSelectedObject() { selected_obj = NULL; }

private:
	TeenAgentEngine *_engine;
	Surface background;
	byte *items;
	uint offset[93];

	Common::Array<InventoryObject> objects;
	byte *inventory;
	struct Item {
		Animation animation;
		Surface surface;
		Rect rect;
		bool hovered;

		Item() : hovered(false) {}
		void free();
		void load(Inventory *inventory, uint item_id);
		void backgroundEffect(Graphics::Surface *s);
		void render(Inventory *inventory, uint item_id, Graphics::Surface *surface, int delta);
	} graphics[24];

	bool _active;
	Common::Point mouse;
	int hovered;
	
	bool tryObjectCallback(InventoryObject *obj);

	InventoryObject *hovered_obj, *selected_obj;
};

} // End of namespace TeenAgent

#endif
