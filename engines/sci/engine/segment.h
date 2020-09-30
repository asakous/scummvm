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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/engines/sci/engine/segment.h $
 * $Id: segment.h 53821 2010-10-25 11:32:23Z thebluegr $
 *
 */

#ifndef SCI_ENGINE_SEGMENT_H
#define SCI_ENGINE_SEGMENT_H

#include "common/serializer.h"
#include "sci/engine/vm.h"
#include "sci/engine/vm_types.h"	// for reg_t
#include "sci/util.h"

namespace Sci {

struct SegmentRef {
	bool isRaw;	///< true if data is raw, false if it is a reg_t sequence
	union {
		byte *raw;
		reg_t *reg;
	};
	int maxSize;	///< number of available bytes

	// FIXME: Perhaps a generic 'offset' is more appropriate here
	bool skipByte; ///< true if referencing the 2nd data byte of *reg, false otherwise

	// TODO: Add this?
	//reg_t pointer;	// Original pointer

	// TODO: Add this?
	//SegmentType type;

	SegmentRef() : isRaw(true), raw(0), maxSize(0), skipByte(false) {}

	bool isValid() const { return (isRaw ? raw != 0 : reg != 0); }
};


enum SegmentType {
	SEG_TYPE_INVALID = 0,
	SEG_TYPE_SCRIPT = 1,
	SEG_TYPE_CLONES = 2,
	SEG_TYPE_LOCALS = 3,
	SEG_TYPE_STACK = 4,
	SEG_TYPE_SYS_STRINGS = 5,
	SEG_TYPE_LISTS = 6,
	SEG_TYPE_NODES = 7,
	SEG_TYPE_HUNK = 8,
	SEG_TYPE_DYNMEM = 9,
	// 10 used to be string fragments, now obsolete

#ifdef ENABLE_SCI32
	SEG_TYPE_ARRAY = 11,
	SEG_TYPE_STRING = 12,
#endif

	SEG_TYPE_MAX // For sanity checking
};

struct SegmentObj : public Common::Serializable {
	SegmentType _type;

public:
	static SegmentObj *createSegmentObj(SegmentType type);
	static const char *getSegmentTypeName(SegmentType type);

public:
	SegmentObj(SegmentType type) : _type(type) {}
	virtual ~SegmentObj() {}

	inline SegmentType getType() const { return _type; }

	/**
	 * Check whether the given offset into this memory object is valid,
	 * i.e., suitable for passing to dereference.
	 */
	virtual bool isValidOffset(uint16 offset) const = 0;

	/**
	 * Dereferences a raw memory pointer.
	 * @param reg	reference to dereference
	 * @return		the data block referenced
	 */
	virtual SegmentRef dereference(reg_t pointer);

	/**
	 * Finds the canonic address associated with sub_reg.
	 * Used by the garbage collector.
	 *
	 * For each valid address a, there exists a canonic address c(a) such that c(a) = c(c(a)).
	 * This address "governs" a in the sense that deallocating c(a) will deallocate a.
	 *
	 * @param sub_addr		base address whose canonic address is to be found
	 */
	virtual reg_t findCanonicAddress(SegManager *segMan, reg_t sub_addr) const { return sub_addr; }

	/**
	 * Deallocates all memory associated with the specified address.
	 * Used by the garbage collector.
	 * @param sub_addr		address (within the given segment) to deallocate
	 */
	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr) {}

	/**
	 * Iterates over and reports all addresses within the segment.
	 * Used by the garbage collector.
	 * @return a list of addresses within the segment
	 */
	virtual Common::Array<reg_t> listAllDeallocatable(SegmentId segId) const {
		return Common::Array<reg_t>();
	}

	/**
	 * Iterates over all references reachable from the specified object.
	 * Used by the garbage collector.
	 * @param  object	object (within the current segment) to analyse
	 * @return a list of outgoing references within the object
	 *
	 * @note This function may also choose to report numbers (segment 0) as adresses
	 */
	virtual Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const {
		return Common::Array<reg_t>();
	}
};

enum {
	SYS_STRINGS_MAX = 4,

	SYS_STRING_SAVEDIR = 0,
	SYS_STRING_PARSER_BASE = 1,

	MAX_PARSER_BASE = 64
};

struct SystemString {
	Common::String _name;
	int _maxSize;
	char *_value;
};

struct SystemStrings : public SegmentObj {
	SystemString _strings[SYS_STRINGS_MAX];

public:
	SystemStrings() : SegmentObj(SEG_TYPE_SYS_STRINGS) {
		for (int i = 0; i < SYS_STRINGS_MAX; i++) {
			_strings[i]._maxSize = 0;
			_strings[i]._value = 0;
		}
	}
	~SystemStrings() {
		for (int i = 0; i < SYS_STRINGS_MAX; i++) {
			SystemString *str = &_strings[i];
			if (!str->_name.empty()) {
				free(str->_value);
				str->_value = NULL;

				str->_maxSize = 0;
			}
		}
	}

	virtual bool isValidOffset(uint16 offset) const;
	virtual SegmentRef dereference(reg_t pointer);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};

struct LocalVariables : public SegmentObj {
	int script_id; /**< Script ID this local variable block belongs to */
	Common::Array<reg_t> _locals;

public:
	LocalVariables(): SegmentObj(SEG_TYPE_LOCALS) {
		script_id = 0;
	}

	virtual bool isValidOffset(uint16 offset) const;
	virtual SegmentRef dereference(reg_t pointer);
	virtual reg_t findCanonicAddress(SegManager *segMan, reg_t sub_addr) const;
	virtual Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const;

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};

/** Clone has been marked as 'freed' */
enum {
	OBJECT_FLAG_FREED = (1 << 0)
};

enum infoSelectorFlags {
	kInfoFlagClone = 0x0001,
	kInfoFlagClass = 0x8000
};

enum ObjectOffsets {
	kOffsetLocalVariables = -6,
	kOffsetFunctionArea = -4,
	kOffsetSelectorCounter = -2,
	kOffsetSelectorSegment = 0,
	kOffsetInfoSelectorSci0 = 4,
	kOffsetNamePointerSci0 = 6,
	kOffsetInfoSelectorSci11 = 14,
	kOffsetNamePointerSci11 = 16
};

class Object {
public:
	Object() {
		_offset = getSciVersion() < SCI_VERSION_1_1 ? 0 : 5;
		_flags = 0;
		_baseObj = 0;
		_baseVars = 0;
		_baseMethod = 0;
		_methodCount = 0;
	}

	~Object() { }

	reg_t getSpeciesSelector() const { return _variables[_offset]; }
	void setSpeciesSelector(reg_t value) { _variables[_offset] = value; }

	reg_t getSuperClassSelector() const { return _variables[_offset + 1]; }
	void setSuperClassSelector(reg_t value) { _variables[_offset + 1] = value; }

	reg_t getInfoSelector() const { return _variables[_offset + 2]; }
	void setInfoSelector(reg_t value) { _variables[_offset + 2] = value; }

	reg_t getNameSelector() const { return _offset + 3 < (uint16)_variables.size() ? _variables[_offset + 3] : NULL_REG; }
	void setNameSelector(reg_t value) { _variables[_offset + 3] = value; }

	reg_t getPropDictSelector() const { return _variables[2]; }
	void setPropDictSelector(reg_t value) { _variables[2] = value; }

	reg_t getClassScriptSelector() const { return _variables[4]; }
	void setClassScriptSelector(reg_t value) { _variables[4] = value; }

	Selector getVarSelector(uint16 i) const { return READ_SCI11ENDIAN_UINT16(_baseVars + i); }

	reg_t getFunction(uint16 i) const {
		uint16 offset = (getSciVersion() < SCI_VERSION_1_1) ? _methodCount + 1 + i : i * 2 + 2;
		return make_reg(_pos.segment, READ_SCI11ENDIAN_UINT16(_baseMethod + offset));
	}

	Selector getFuncSelector(uint16 i) const {
		uint16 offset = (getSciVersion() < SCI_VERSION_1_1) ? i : i * 2 + 1;
		return READ_SCI11ENDIAN_UINT16(_baseMethod + offset);
	}

	/**
	 * Determines if this object is a class and explicitly defines the
	 * selector as a funcselector. Does NOT say anything about the object's
	 * superclasses, i.e. failure may be returned even if one of the
	 * superclasses defines the funcselector
	 */
	int funcSelectorPosition(Selector sel) const {
		for (uint i = 0; i < _methodCount; i++)
			if (getFuncSelector(i) == sel)
				return i;

		return -1;
	}

	/**
	 * Determines if the object explicitly defines slc as a varselector.
	 * Returns -1 if not found.
	 */
	int locateVarSelector(SegManager *segMan, Selector slc) const;

	bool isClass() const { return (getInfoSelector().offset & kInfoFlagClass); }
	const Object *getClass(SegManager *segMan) const;

	void markAsFreed() { _flags |= OBJECT_FLAG_FREED; }
	bool isFreed() const { return _flags & OBJECT_FLAG_FREED; }

	uint getVarCount() const { return _variables.size(); }

	void init(byte *buf, reg_t obj_pos, bool initVariables = true);

	reg_t getVariable(uint var) const { return _variables[var]; }
	reg_t &getVariableRef(uint var) { return _variables[var]; }

	uint16 getMethodCount() const { return _methodCount; }
	reg_t getPos() const { return _pos; }

	void saveLoadWithSerializer(Common::Serializer &ser);

	void cloneFromObject(const Object *obj) {
		_baseObj = obj ? obj->_baseObj : NULL;
		_baseMethod = obj ? obj->_baseMethod : NULL;
		_baseVars = obj ? obj->_baseVars : NULL;
	}

	bool relocate(SegmentId segment, int location, size_t scriptSize);

	int propertyOffsetToId(SegManager *segMan, int propertyOffset) const;

	void initSpecies(SegManager *segMan, reg_t addr);
	void initSuperClass(SegManager *segMan, reg_t addr);
	bool initBaseObject(SegManager *segMan, reg_t addr, bool doInitSuperClass = true);

	// TODO: make private
	// Only SegManager::reconstructScripts() is left needing direct access to these
public:
	const byte *_baseObj; /**< base + object offset within base */

private:
	const uint16 *_baseVars; /**< Pointer to the varselector area for this object */
	const uint16 *_baseMethod; /**< Pointer to the method selector area for this object */

	Common::Array<reg_t> _variables;
	uint16 _methodCount;
	int _flags;
	uint16 _offset;
	reg_t _pos; /**< Object offset within its script; for clones, this is their base */
};

/** Data stack */
struct DataStack : SegmentObj {
	int _capacity; /**< Number of stack entries */
	reg_t *_entries;

public:
	DataStack() : SegmentObj(SEG_TYPE_STACK) {
		_capacity = 0;
		_entries = NULL;
	}
	~DataStack() {
		free(_entries);
		_entries = NULL;
	}

	virtual bool isValidOffset(uint16 offset) const;
	virtual SegmentRef dereference(reg_t pointer);
	virtual reg_t findCanonicAddress(SegManager *segMan, reg_t sub_addr) const;
	virtual Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const;

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};

enum {
	CLONE_USED = -1,
	CLONE_NONE = -1
};

typedef Object Clone;

struct Node {
	reg_t pred; /**< Predecessor node */
	reg_t succ; /**< Successor node */
	reg_t key;
	reg_t value;
}; /* List nodes */

struct List {
	reg_t first;
	reg_t last;
};

struct Hunk {
	void *mem;
	unsigned int size;
	const char *type;
};

template<typename T>
struct Table : public SegmentObj {
	typedef T value_type;
	struct Entry : public T {
		int next_free; /* Only used for free entries */
	};
	enum { HEAPENTRY_INVALID = -1 };


	int first_free; /**< Beginning of a singly linked list for entries */
	int entries_used; /**< Statistical information */

	Common::Array<Entry> _table;

public:
	Table(SegmentType type) : SegmentObj(type) {
		initTable();
	}

	void initTable() {
		entries_used = 0;
		first_free = HEAPENTRY_INVALID;
		_table.clear();
	}

	int allocEntry() {
		entries_used++;
		if (first_free != HEAPENTRY_INVALID) {
			int oldff = first_free;
			first_free = _table[oldff].next_free;

			_table[oldff].next_free = oldff;
			return oldff;
		} else {
			uint newIdx = _table.size();
			_table.push_back(Entry());
			_table[newIdx].next_free = newIdx;	// Tag as 'valid'
			return newIdx;
		}
	}

	virtual bool isValidOffset(uint16 offset) const {
		return isValidEntry(offset);
	}

	bool isValidEntry(int idx) const {
		return idx >= 0 && (uint)idx < _table.size() && _table[idx].next_free == idx;
	}

	virtual void freeEntry(int idx) {
		if (idx < 0 || (uint)idx >= _table.size())
			::error("Table::freeEntry: Attempt to release invalid table index %d", idx);

		_table[idx].next_free = first_free;
		first_free = idx;
		entries_used--;
	}

	virtual Common::Array<reg_t> listAllDeallocatable(SegmentId segId) const {
		Common::Array<reg_t> tmp;
		for (uint i = 0; i < _table.size(); i++)
			if (isValidEntry(i))
				tmp.push_back(make_reg(segId, i));
		return tmp;
	}
};


/* CloneTable */
struct CloneTable : public Table<Clone> {
	CloneTable() : Table<Clone>(SEG_TYPE_CLONES) {}

	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr);
	virtual Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const;

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


/* NodeTable */
struct NodeTable : public Table<Node> {
	NodeTable() : Table<Node>(SEG_TYPE_NODES) {}

	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr);
	virtual Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const;

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


/* ListTable */
struct ListTable : public Table<List> {
	ListTable() : Table<List>(SEG_TYPE_LISTS) {}

	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr);
	virtual Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const;

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


/* HunkTable */
struct HunkTable : public Table<Hunk> {
	HunkTable() : Table<Hunk>(SEG_TYPE_HUNK) {}

	virtual void freeEntry(int idx) {
		Table<Hunk>::freeEntry(idx);

		if (!_table[idx].mem)
			warning("Attempt to free an already freed hunk");
		free(_table[idx].mem);
		_table[idx].mem = 0;
	}

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


// Free-style memory
struct DynMem : public SegmentObj {
	int _size;
	Common::String _description;
	byte *_buf;

public:
	DynMem() : SegmentObj(SEG_TYPE_DYNMEM), _size(0), _buf(0) {}
	~DynMem() {
		free(_buf);
		_buf = NULL;
	}

	virtual bool isValidOffset(uint16 offset) const;
	virtual SegmentRef dereference(reg_t pointer);
	virtual reg_t findCanonicAddress(SegManager *segMan, reg_t sub_addr) const;
	virtual Common::Array<reg_t> listAllDeallocatable(SegmentId segId) const;

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};

#ifdef ENABLE_SCI32

template <typename T>
class SciArray {
public:
	SciArray() {
		_type = -1;
		_data = NULL;
		_size = 0;
		_actualSize = 0;
	}

	SciArray(const SciArray<T> &array) {
		_type = array._type;
		_size = array._size;
		_actualSize = array._actualSize;
		_data = new T[_actualSize];
		assert(_data);
		memcpy(_data, array._data, _size * sizeof(T));
	}

	SciArray<T>& operator=(const SciArray<T> &array) {
		if (this == &array)
			return *this;

		delete[] _data;
		_type = array._type;
		_size = array._size;
		_actualSize = array._actualSize;
		_data = new T[_actualSize];
		assert(_data);
		memcpy(_data, array._data, _size * sizeof(T));

		return *this;
	}

	virtual ~SciArray() {
		destroy();
	}

	virtual void destroy() {
		delete[] _data;
		_data = NULL;
		_type = -1;
		_size = _actualSize = 0;
	}

	void setType(byte type) {
		if (_type >= 0)
			error("SciArray::setType(): Type already set");

		_type = type;
	}

	void setSize(uint32 size) {
		if (_type < 0)
			error("SciArray::setSize(): No type set");

		// Check if we don't have to do anything
		if (_size == size)
			return;

		// Check if we don't have to expand the array
		if (size <= _actualSize) {
			_size = size;
			return;
		}

		// So, we're going to have to create an array of some sort
		T *newArray = new T[size];
		memset(newArray, 0, size * sizeof(T));

		// Check if we never created an array before
		if (!_data) {
			_size = _actualSize = size;
			_data = newArray;
			return;
		}

		// Copy data from the old array to the new
		memcpy(newArray, _data, _size * sizeof(T));

		// Now set the new array to the old and set the sizes
		delete[] _data;
		_data = newArray;
		_size = _actualSize = size;
	}

	T getValue(uint16 index) const {
		if (index >= _size)
			error("SciArray::getValue(): %d is out of bounds (%d)", index, _size);

		return _data[index];
	}

	void setValue(uint16 index, T value) {
		if (index >= _size)
			error("SciArray::setValue(): %d is out of bounds (%d)", index, _size);

		_data[index] = value;
	}

	byte getType() const { return _type; }
	uint32 getSize() const { return _size; }
	T *getRawData() { return _data; }
	const T *getRawData() const { return _data; }

protected:
	int8 _type;
	T *_data;
	uint32 _size; // _size holds the number of entries that the scripts have requested
	uint32 _actualSize; // _actualSize is the actual numbers of entries allocated
};

class SciString : public SciArray<char> {
public:
	SciString() : SciArray<char>() { setType(3); }

	// We overload destroy to ensure the string type is 3 after destroying
	void destroy() { SciArray<char>::destroy(); _type = 3; }

	Common::String toString() const;
	void fromString(const Common::String &string);
};

struct ArrayTable : public Table<SciArray<reg_t> > {
	ArrayTable() : Table<SciArray<reg_t> >(SEG_TYPE_ARRAY) {}

	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr);
	virtual Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const;

	void saveLoadWithSerializer(Common::Serializer &ser);
	SegmentRef dereference(reg_t pointer);
};

struct StringTable : public Table<SciString> {
	StringTable() : Table<SciString>(SEG_TYPE_STRING) {}

	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr);

	void saveLoadWithSerializer(Common::Serializer &ser);
	SegmentRef dereference(reg_t pointer);
};

#endif


} // End of namespace Sci

#endif // SCI_ENGINE_SEGMENT_H
