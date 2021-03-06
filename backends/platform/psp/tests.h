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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/psp/psp_main.cpp $
 * $Id: psp_main.cpp 49155 2010-05-23 11:48:21Z Bluddy $
 *
 */

#ifndef _PSP_TESTS_H_ 
#define _PSP_TESTS_H_

//#define PSP_ENABLE_UNIT_TESTS		// run unit tests
//#define PSP_ENABLE_SPEED_TESTS		// run speed tests

#if defined (PSP_ENABLE_UNIT_TESTS) || defined (PSP_ENABLE_SPEED_TESTS)	
void psp_tests();
#endif

#endif /* _PSP_TESTS_H_ */