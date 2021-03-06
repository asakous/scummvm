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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/backends/fs/windows/windows-fs.cpp $
 * $Id: windows-fs.cpp 36014 2009-01-23 03:41:36Z fingolfin $
 */

#ifdef WIN32

#if defined(ARRAYSIZE)
#undef ARRAYSIZE
#endif
#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one...
#undef ARRAYSIZE
#ifdef _WIN32_WCE
#undef GetCurrentDirectory
#endif
#include "backends/fs/abstract-fs.h"
#include "backends/fs/stdiostream.h"
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

// F_OK, R_OK and W_OK are not defined under MSVC, so we define them here
// For more information on the modes used by MSVC, check:
// http://msdn2.microsoft.com/en-us/library/1w06ktdy(VS.80).aspx
#ifndef F_OK
#define F_OK 0
#endif

#ifndef R_OK
#define R_OK 4
#endif

#ifndef W_OK
#define W_OK 2
#endif

/**
 * Implementation of the ScummVM file system API based on Windows API.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class WindowsFilesystemNode : public AbstractFSNode {
protected:
	Common::String _displayName;
	Common::String _path;
	bool _isDirectory;
	bool _isPseudoRoot;
	bool _isValid;

public:
	/**
	 * Creates a WindowsFilesystemNode with the root node as path.
	 *
	 * In regular windows systems, a virtual root path is used "".
	 * In windows CE, the "\" root is used instead.
	 */
	WindowsFilesystemNode();

	/**
	 * Creates a WindowsFilesystemNode for a given path.
	 *
	 * Examples:
	 *			path=c:\foo\bar.txt, currentDir=false -> c:\foo\bar.txt
	 *			path=c:\foo\bar.txt, currentDir=true -> current directory
	 *			path=NULL, currentDir=true -> current directory
	 *
	 * @param path Common::String with the path the new node should point to.
	 * @param currentDir if true, the path parameter will be ignored and the resulting node will point to the current directory.
	 */
	WindowsFilesystemNode(const Common::String &path, const bool currentDir);

	virtual bool exists() const { return _access(_path.c_str(), F_OK) == 0; }
	virtual Common::String getDisplayName() const { return _displayName; }
	virtual Common::String getName() const { return _displayName; }
	virtual Common::String getPath() const { return _path; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual bool isReadable() const { return _access(_path.c_str(), R_OK) == 0; }
	virtual bool isWritable() const { return _access(_path.c_str(), W_OK) == 0; }

	virtual AbstractFSNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *createReadStream();
	virtual Common::WriteStream *createWriteStream();

private:
	/**
	 * Adds a single WindowsFilesystemNode to a given list.
	 * This method is used by getChildren() to populate the directory entries list.
	 *
	 * @param list List to put the file entry node in.
	 * @param mode Mode to use while adding the file entry to the list.
	 * @param base Common::String with the directory being listed.
	 * @param find_data Describes a file that the FindFirstFile, FindFirstFileEx, or FindNextFile functions find.
	 */
	static void addFile(AbstractFSList &list, ListMode mode, const char *base, WIN32_FIND_DATA* find_data);

	/**
	 * Converts a Unicode string to Ascii format.
	 *
	 * @param str Common::String to convert from Unicode to Ascii.
	 * @return str in Ascii format.
	 */
	static char *toAscii(TCHAR *str);

	/**
	 * Converts an Ascii string to Unicode format.
	 *
	 * @param str Common::String to convert from Ascii to Unicode.
	 * @return str in Unicode format.
	 */
	static const TCHAR* toUnicode(const char *str);
};

void WindowsFilesystemNode::addFile(AbstractFSList &list, ListMode mode, const char *base, WIN32_FIND_DATA* find_data) {
	WindowsFilesystemNode entry;
	char *asciiName = toAscii(find_data->cFileName);
	bool isDirectory;

	// Skip local directory (.) and parent (..)
	if (!strcmp(asciiName, ".") || !strcmp(asciiName, ".."))
		return;

	isDirectory = (find_data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? true : false);

	if ((!isDirectory && mode == Common::FSNode::kListDirectoriesOnly) ||
		(isDirectory && mode == Common::FSNode::kListFilesOnly))
		return;

	entry._isDirectory = isDirectory;
	entry._displayName = asciiName;
	entry._path = base;
	entry._path += asciiName;
	if (entry._isDirectory)
		entry._path += "\\";
	entry._isValid = true;
	entry._isPseudoRoot = false;

	list.push_back(new WindowsFilesystemNode(entry));
}

char* WindowsFilesystemNode::toAscii(TCHAR *str) {
#ifndef UNICODE
	return (char*)str;
#else
	static char asciiString[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, str, _tcslen(str) + 1, asciiString, sizeof(asciiString), NULL, NULL);
	return asciiString;
#endif
}

const TCHAR* WindowsFilesystemNode::toUnicode(const char *str) {
#ifndef UNICODE
	return (const TCHAR *)str;
#else
	static TCHAR unicodeString[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, str, strlen(str) + 1, unicodeString, sizeof(unicodeString) / sizeof(TCHAR));
	return unicodeString;
#endif
}

WindowsFilesystemNode::WindowsFilesystemNode() {
	_isDirectory = true;
#ifndef _WIN32_WCE
	// Create a virtual root directory for standard Windows system
	_isValid = false;
	_path = "";
	_isPseudoRoot = true;
#else
	_displayName = "Root";
	// No need to create a pseudo root directory on Windows CE
	_isValid = true;
	_path = "\\";
	_isPseudoRoot = false;
#endif
}

WindowsFilesystemNode::WindowsFilesystemNode(const Common::String &p, const bool currentDir) {
	if (currentDir) {
		char path[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, path);
		_path = path;
	} else {
		assert(p.size() > 0);
		_path = p;
	}

	_displayName = lastPathComponent(_path, '\\');

	// Check whether it is a directory, and whether the file actually exists
	DWORD fileAttribs = GetFileAttributes(toUnicode(_path.c_str()));

	if (fileAttribs == INVALID_FILE_ATTRIBUTES) {
		_isDirectory = false;
		_isValid = false;
	} else {
		_isDirectory = ((fileAttribs & FILE_ATTRIBUTE_DIRECTORY) != 0);
		_isValid = true;
		// Add a trailing slash, if necessary.
		if (_isDirectory && _path.lastChar() != '\\') {
			_path += '\\';
		}
	}
	_isPseudoRoot = false;
}

AbstractFSNode *WindowsFilesystemNode::getChild(const Common::String &n) const {
	assert(_isDirectory);

	// Make sure the string contains no slashes
	assert(!n.contains('/'));

	Common::String newPath(_path);
	if (_path.lastChar() != '\\')
		newPath += '\\';
	newPath += n;

	return new WindowsFilesystemNode(newPath, false);
}

bool WindowsFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	assert(_isDirectory);

	//TODO: honor the hidden flag

	if (_isPseudoRoot) {
#ifndef _WIN32_WCE
		// Drives enumeration
		TCHAR drive_buffer[100];
		GetLogicalDriveStrings(sizeof(drive_buffer) / sizeof(TCHAR), drive_buffer);

		for (TCHAR *current_drive = drive_buffer; *current_drive;
			current_drive += _tcslen(current_drive) + 1) {
				WindowsFilesystemNode entry;
				char drive_name[2];

				drive_name[0] = toAscii(current_drive)[0];
				drive_name[1] = '\0';
				entry._displayName = drive_name;
				entry._isDirectory = true;
				entry._isValid = true;
				entry._isPseudoRoot = false;
				entry._path = toAscii(current_drive);
				myList.push_back(new WindowsFilesystemNode(entry));
		}
#endif
	}
	else {
		// Files enumeration
		WIN32_FIND_DATA desc;
		HANDLE handle;
		char searchPath[MAX_PATH + 10];

		sprintf(searchPath, "%s*", _path.c_str());

		handle = FindFirstFile(toUnicode(searchPath), &desc);

		if (handle == INVALID_HANDLE_VALUE)
			return false;

		addFile(myList, mode, _path.c_str(), &desc);

		while (FindNextFile(handle, &desc))
			addFile(myList, mode, _path.c_str(), &desc);

		FindClose(handle);
	}

	return true;
}

AbstractFSNode *WindowsFilesystemNode::getParent() const {
	assert(_isValid || _isPseudoRoot);

	if (_isPseudoRoot)
		return 0;

	WindowsFilesystemNode *p = new WindowsFilesystemNode();
	if (_path.size() > 3) {
		const char *start = _path.c_str();
		const char *end = lastPathComponent(_path, '\\');

		p = new WindowsFilesystemNode();
		p->_path = Common::String(start, end - start);
		p->_isValid = true;
		p->_isDirectory = true;
		p->_displayName = lastPathComponent(p->_path, '\\');
		p->_isPseudoRoot = false;
	}

	return p;
}

Common::SeekableReadStream *WindowsFilesystemNode::createReadStream() {
	return StdioStream::makeFromPath(getPath().c_str(), false);
}

Common::WriteStream *WindowsFilesystemNode::createWriteStream() {
	return StdioStream::makeFromPath(getPath().c_str(), true);
}

#endif //#ifdef WIN32
