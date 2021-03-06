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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-2-1/graphics/video/qt_decoder.cpp $
 * $Id: qt_decoder.cpp 52603 2010-09-06 18:15:51Z mthreepwood $
 *
 */

//
// Heavily based on ffmpeg code.
//
// Copyright (c) 2001 Fabrice Bellard.
// First version by Francois Revol revol@free.fr
// Seek function by Gael Chardon gael.dev@4now.net
//

#include "graphics/video/qt_decoder.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/macresman.h"
#include "common/util.h"
#include "common/zlib.h"

// Audio codecs
#include "sound/decoders/adpcm.h"
#include "sound/decoders/raw.h"
#include "graphics/video/codecs/qdm2.h"

// Video codecs
#include "graphics/video/codecs/cinepak.h"
#include "graphics/video/codecs/mjpeg.h"
#include "graphics/video/codecs/qtrle.h"
#include "graphics/video/codecs/rpza.h"
#include "graphics/video/codecs/smc.h"

namespace Graphics {

////////////////////////////////////////////
// QuickTimeDecoder
////////////////////////////////////////////

QuickTimeDecoder::QuickTimeDecoder() : VideoDecoder() {
	_audStream = NULL;
	_beginOffset = 0;
	_videoCodec = NULL;
	_curFrame = -1;
	_startTime = _nextFrameStartTime = 0;
	_audHandle = Audio::SoundHandle();
	_numStreams = 0;
	_fd = 0;
	_scaledSurface = 0;
	_scaleFactorX = 1;
	_scaleFactorY = 1;
	_dirtyPalette = false;
	_resFork = new Common::MacResManager();

	initParseTable();
}

QuickTimeDecoder::~QuickTimeDecoder() {
	close();
	delete _resFork;
}

uint16 QuickTimeDecoder::getWidth() const {
	if (_videoStreamIndex < 0)
		return 0;

	return (Common::Rational(_streams[_videoStreamIndex]->width) / getScaleFactorX()).toInt();
}

uint16 QuickTimeDecoder::getHeight() const {
	if (_videoStreamIndex < 0)
		return 0;

	return (Common::Rational(_streams[_videoStreamIndex]->height) / getScaleFactorY()).toInt();
}

uint32 QuickTimeDecoder::getFrameCount() const {
	if (_videoStreamIndex < 0)
		return 0;

	return _streams[_videoStreamIndex]->nb_frames;
}

byte QuickTimeDecoder::getBitsPerPixel() {
	if (_videoStreamIndex < 0)
		return 0;

	return _streams[_videoStreamIndex]->bits_per_sample & 0x1F;
}

uint32 QuickTimeDecoder::getCodecTag() {
	if (_videoStreamIndex < 0)
		return 0;

	return _streams[_videoStreamIndex]->codec_tag;
}

Common::Rational QuickTimeDecoder::getScaleFactorX() const {
	if (_videoStreamIndex < 0)
		return 1;

	return (_scaleFactorX * _streams[_videoStreamIndex]->scaleFactorX);
}

Common::Rational QuickTimeDecoder::getScaleFactorY() const {
	if (_videoStreamIndex < 0)
		return 1;

	return (_scaleFactorY * _streams[_videoStreamIndex]->scaleFactorY);
}

uint32 QuickTimeDecoder::getFrameDuration() {
	if (_videoStreamIndex < 0)
		return 0;

	uint32 curFrameIndex = 0;
	for (int32 i = 0; i < _streams[_videoStreamIndex]->stts_count; i++) {
		curFrameIndex += _streams[_videoStreamIndex]->stts_data[i].count;
		if ((uint32)_curFrame < curFrameIndex) {
			// Ok, now we have what duration this frame has.
			return _streams[_videoStreamIndex]->stts_data[i].duration;
		}
	}

	// This should never occur
	error ("Cannot find duration for frame %d", _curFrame);
	return 0;
}

PixelFormat QuickTimeDecoder::getPixelFormat() const {
	if (!_videoCodec)
		return PixelFormat::createFormatCLUT8();

	return _videoCodec->getPixelFormat();
}

void QuickTimeDecoder::rewind() {
	VideoDecoder::reset();
	_nextFrameStartTime = 0;

	// Restart the audio too
	stopAudio();
	if (_audioStreamIndex >= 0) {
		_curAudioChunk = 0;
		_audStream = Audio::makeQueuingAudioStream(_streams[_audioStreamIndex]->sample_rate, _streams[_audioStreamIndex]->channels == 2);
	}
	startAudio();
}

Codec *QuickTimeDecoder::createCodec(uint32 codecTag, byte bitsPerPixel) {
	if (codecTag == MKID_BE('cvid')) {
		// Cinepak: As used by most Myst and all Riven videos as well as some Myst ME videos. "The Chief" videos also use this.
		return new CinepakDecoder();
	} else if (codecTag == MKID_BE('rpza')) {
		// Apple Video ("Road Pizza"): Used by some Myst videos.
		return new RPZADecoder(getWidth(), getHeight());
	} else if (codecTag == MKID_BE('rle ')) {
		// QuickTime RLE: Used by some Myst ME videos.
		return new QTRLEDecoder(getWidth(), getHeight(), bitsPerPixel);
	} else if (codecTag == MKID_BE('smc ')) {
		// Apple SMC: Used by some Myst videos.
		return new SMCDecoder(getWidth(), getHeight());
	} else if (codecTag == MKID_BE('SVQ1')) {
		// Sorenson Video 1: Used by some Myst ME videos.
		warning ("Sorenson Video 1 not yet supported");
	} else if (codecTag == MKID_BE('SVQ3')) {
		// Sorenson Video 3: Used by some Myst ME videos.
		warning ("Sorenson Video 3 not yet supported");
	} else if (codecTag == MKID_BE('jpeg')) {
		// Motion JPEG: Used by some Myst ME 10th Anniversary videos.
		return new JPEGDecoder();
	} else if (codecTag == MKID_BE('QkBk')) {
		// CDToons: Used by most of the Broderbund games. This is an unknown format so far.
		warning ("CDToons not yet supported");
	} else {
		warning ("Unsupported codec \'%s\'", tag2str(codecTag));
	}

	return NULL;
}

void QuickTimeDecoder::startAudio() {
	if (_audStream) { // No audio/audio not supported
		updateAudioBuffer();
		g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_audHandle, _audStream);
	}
}

void QuickTimeDecoder::stopAudio() {
	if (_audStream) {
		g_system->getMixer()->stopHandle(_audHandle);
		_audStream = NULL; // the mixer automatically frees the stream
	}
}

void QuickTimeDecoder::pauseVideoIntern(bool pause) {
	if (_audStream)
		g_system->getMixer()->pauseHandle(_audHandle, pause);
}

Surface *QuickTimeDecoder::decodeNextFrame() {
	if (!_videoCodec || _curFrame >= (int32)getFrameCount() - 1)
		return NULL;

	if (_startTime == 0)
		_startTime = g_system->getMillis();

	_curFrame++;
	_nextFrameStartTime += getFrameDuration();

	Common::SeekableReadStream *frameData = getNextFramePacket();

	if (frameData) {
		Surface *frame = _videoCodec->decodeImage(frameData);
		delete frameData;
		return scaleSurface(frame);
	}

	return NULL;
}

Surface *QuickTimeDecoder::scaleSurface(Surface *frame) {
	if (getScaleFactorX() == 1 && getScaleFactorY() == 1)
		return frame;

	assert(_scaledSurface);

	for (int32 j = 0; j < _scaledSurface->h; j++)
		for (int32 k = 0; k < _scaledSurface->w; k++)
			memcpy(_scaledSurface->getBasePtr(k, j), frame->getBasePtr((k * getScaleFactorX()).toInt() , (j * getScaleFactorY()).toInt()), frame->bytesPerPixel);

	return _scaledSurface;
}

bool QuickTimeDecoder::endOfVideo() const {
	return (!_audStream || _audStream->endOfData()) && (!_videoCodec || VideoDecoder::endOfVideo());
}

uint32 QuickTimeDecoder::getElapsedTime() const {
	if (_audStream)
		return g_system->getMixer()->getSoundElapsedTime(_audHandle);

	return g_system->getMillis() - _startTime;
}

uint32 QuickTimeDecoder::getTimeToNextFrame() const {
	if (endOfVideo() || _curFrame < 0)
		return 0;

	// Convert from the QuickTime rate base to 1000
	uint32 nextFrameStartTime = _nextFrameStartTime * 1000 / _streams[_videoStreamIndex]->time_scale;
	uint32 elapsedTime = getElapsedTime();

	if (nextFrameStartTime <= elapsedTime)
		return 0;

	return nextFrameStartTime - elapsedTime;
}

bool QuickTimeDecoder::loadFile(const Common::String &filename) {
	if (!_resFork->open(filename) || !_resFork->hasDataFork())
		return false;

	_foundMOOV = _foundMDAT = false;
	_numStreams = 0;
	_partial = 0;
	_videoStreamIndex = _audioStreamIndex = -1;
	_startTime = 0;

	MOVatom atom = { 0, 0, 0xffffffff };

	if (_resFork->hasResFork()) {
		// Search for a 'moov' resource
		Common::MacResIDArray idArray = _resFork->getResIDArray(MKID_BE('moov'));

		if (!idArray.empty())
			_fd = _resFork->getResource(MKID_BE('moov'), idArray[0]);

		if (_fd) {
			atom.size = _fd->size();
			if (readDefault(atom) < 0 || !_foundMOOV)
				return false;
		}
		delete _fd;

		atom.type = 0;
		atom.offset = 0;
		atom.size = 0xffffffff;
	}

	_fd = _resFork->getDataFork();

	if (readDefault(atom) < 0 || !_foundMOOV || !_foundMDAT)
		return false;

	init();
	return true;
}

bool QuickTimeDecoder::load(Common::SeekableReadStream *stream) {
	_fd = stream;
	_foundMOOV = _foundMDAT = false;
	_numStreams = 0;
	_partial = 0;
	_videoStreamIndex = _audioStreamIndex = -1;
	_startTime = 0;

	MOVatom atom = { 0, 0, 0xffffffff };

	if (readDefault(atom) < 0 || !_foundMOOV || !_foundMDAT) {
		_fd = 0;
		return false;
	}

	init();
	return true;
}

void QuickTimeDecoder::init() {
	// some cleanup : make sure we are on the mdat atom
	if((uint32)_fd->pos() != _mdatOffset)
		_fd->seek(_mdatOffset, SEEK_SET);

	for (uint32 i = 0; i < _numStreams;) {
		if (_streams[i]->codec_type == CODEC_TYPE_MOV_OTHER) {// not audio, not video, delete
			delete _streams[i];
			for (uint32 j = i + 1; j < _numStreams; j++)
				_streams[j - 1] = _streams[j];
			_numStreams--;
		} else
			i++;
	}

	for (uint32 i = 0; i < _numStreams; i++) {
		MOVStreamContext *sc = _streams[i];

		if (!sc->time_rate)
			sc->time_rate = 1;

		if (!sc->time_scale)
			sc->time_scale = _timeScale;

		sc->duration /= sc->time_rate;

		sc->ffindex = i;
		sc->is_ff_stream = 1;

		if (sc->codec_type == CODEC_TYPE_VIDEO && _videoStreamIndex < 0)
			_videoStreamIndex = i;
		else if (sc->codec_type == CODEC_TYPE_AUDIO && _audioStreamIndex < 0)
			_audioStreamIndex = i;
	}

	if (_audioStreamIndex >= 0 && checkAudioCodecSupport(_streams[_audioStreamIndex]->codec_tag)) {
		_audStream = Audio::makeQueuingAudioStream(_streams[_audioStreamIndex]->sample_rate, _streams[_audioStreamIndex]->channels == 2);
		_curAudioChunk = 0;

		// Make sure the bits per sample transfers to the sample size
		if (_streams[_audioStreamIndex]->codec_tag == MKID_BE('raw ') || _streams[_audioStreamIndex]->codec_tag == MKID_BE('twos'))
			_streams[_audioStreamIndex]->sample_size = (_streams[_audioStreamIndex]->bits_per_sample / 8) * _streams[_audioStreamIndex]->channels;

		startAudio();
	}

	if (_videoStreamIndex >= 0) {
		_videoCodec = createCodec(getCodecTag(), getBitsPerPixel());

		if (getScaleFactorX() != 1 || getScaleFactorY() != 1) {
			// We have to initialize the scaled surface
			_scaledSurface = new Surface();
			_scaledSurface->create(getWidth(), getHeight(), getPixelFormat().bytesPerPixel);
		}
	}
}

void QuickTimeDecoder::initParseTable() {
	static const ParseTable p[] = {
		{ &QuickTimeDecoder::readDefault, MKID_BE('dinf') },
		{ &QuickTimeDecoder::readLeaf,    MKID_BE('dref') },
		{ &QuickTimeDecoder::readDefault, MKID_BE('edts') },
		{ &QuickTimeDecoder::readELST,    MKID_BE('elst') },
		{ &QuickTimeDecoder::readHDLR,    MKID_BE('hdlr') },
		{ &QuickTimeDecoder::readMDAT,    MKID_BE('mdat') },
		{ &QuickTimeDecoder::readMDHD,    MKID_BE('mdhd') },
		{ &QuickTimeDecoder::readDefault, MKID_BE('mdia') },
		{ &QuickTimeDecoder::readDefault, MKID_BE('minf') },
		{ &QuickTimeDecoder::readMOOV,    MKID_BE('moov') },
		{ &QuickTimeDecoder::readMVHD,    MKID_BE('mvhd') },
		{ &QuickTimeDecoder::readLeaf,    MKID_BE('smhd') },
		{ &QuickTimeDecoder::readDefault, MKID_BE('stbl') },
		{ &QuickTimeDecoder::readSTCO,    MKID_BE('stco') },
		{ &QuickTimeDecoder::readSTSC,    MKID_BE('stsc') },
		{ &QuickTimeDecoder::readSTSD,    MKID_BE('stsd') },
		{ &QuickTimeDecoder::readSTSS,    MKID_BE('stss') },
		{ &QuickTimeDecoder::readSTSZ,    MKID_BE('stsz') },
		{ &QuickTimeDecoder::readSTTS,    MKID_BE('stts') },
		{ &QuickTimeDecoder::readTKHD,    MKID_BE('tkhd') },
		{ &QuickTimeDecoder::readTRAK,    MKID_BE('trak') },
		{ &QuickTimeDecoder::readLeaf,    MKID_BE('udta') },
		{ &QuickTimeDecoder::readLeaf,    MKID_BE('vmhd') },
		{ &QuickTimeDecoder::readCMOV,    MKID_BE('cmov') },
		{ &QuickTimeDecoder::readWAVE,    MKID_BE('wave') },
		{ 0, 0 }
	};

	_parseTable = p;
}

int QuickTimeDecoder::readDefault(MOVatom atom) {
	uint32 total_size = 0;
	MOVatom a;
	int err = 0;

	a.offset = atom.offset;

	while(((total_size + 8) < atom.size) && !_fd->eos() && _fd->pos() < _fd->size() && !err) {
		a.size = atom.size;
		a.type = 0;

		if (atom.size >= 8) {
			a.size = _fd->readUint32BE();
			a.type = _fd->readUint32BE();

			// Some QuickTime videos with resource forks have mdat chunks
			// that are of size 0. Adjust it so it's the correct size.
			if (a.type == MKID_BE('mdat') && a.size == 0)
				a.size = _fd->size();
		}

		total_size += 8;
		a.offset += 8;
		debug(4, "type: %08x  %.4s  sz: %x %x %x", a.type, tag2str(a.type), a.size, atom.size, total_size);

		if (a.size == 1) { // 64 bit extended size
			warning("64 bit extended size is not supported in QuickTime");
			return -1;
		}

		if (a.size == 0) {
			a.size = atom.size - total_size;
			if (a.size <= 8)
				break;
		}

		uint32 i = 0;

		for (; _parseTable[i].type != 0 && _parseTable[i].type != a.type; i++)
			// empty;

		if (a.size < 8)
			break;

		a.size -= 8;

		if (_parseTable[i].type == 0) { // skip leaf atoms data
			debug(0, ">>> Skipped [%s]", tag2str(a.type));

			_fd->seek(a.size, SEEK_CUR);
		} else {
			uint32 start_pos = _fd->pos();
			err = (this->*_parseTable[i].func)(a);

			uint32 left = a.size - _fd->pos() + start_pos;

			if (left > 0) // skip garbage at atom end
				_fd->seek(left, SEEK_CUR);
		}

		a.offset += a.size;
		total_size += a.size;
	}

	if (!err && total_size < atom.size)
		_fd->seek(atom.size - total_size, SEEK_SET);

	return err;
}

int QuickTimeDecoder::readLeaf(MOVatom atom) {
	if (atom.size > 1)
		_fd->seek(atom.size, SEEK_SET);

	return 0;
}

int QuickTimeDecoder::readMOOV(MOVatom atom) {
	if (readDefault(atom) < 0)
		return -1;

	// we parsed the 'moov' atom, we can terminate the parsing as soon as we find the 'mdat'
	// so we don't parse the whole file if over a network
	_foundMOOV = true;

	if(_foundMDAT)
		return 1; // found both, just go

	return 0; // now go for mdat
}

int QuickTimeDecoder::readCMOV(MOVatom atom) {
#ifdef USE_ZLIB
	// Read in the dcom atom
	_fd->readUint32BE();
	if (_fd->readUint32BE() != MKID_BE('dcom'))
		return -1;
	if (_fd->readUint32BE() != MKID_BE('zlib')) {
		warning("Unknown cmov compression type");
		return -1;
	}

	// Read in the cmvd atom
	uint32 compressedSize = _fd->readUint32BE() - 12;
	if (_fd->readUint32BE() != MKID_BE('cmvd'))
		return -1;
	uint32 uncompressedSize = _fd->readUint32BE();

	// Read in data
	byte *compressedData = (byte *)malloc(compressedSize);
	_fd->read(compressedData, compressedSize);

	// Create uncompressed stream
	byte *uncompressedData = (byte *)malloc(uncompressedSize);

	// Uncompress the data
	unsigned long dstLen = uncompressedSize;
	if (!Common::uncompress(uncompressedData, &dstLen, compressedData, compressedSize)) {
		warning ("Could not uncompress cmov chunk");
		free(compressedData);
		free(uncompressedData);
		return -1;
	}

	// Load data into a new MemoryReadStream and assign _fd to be that
	Common::SeekableReadStream *oldStream = _fd;
	_fd = new Common::MemoryReadStream(uncompressedData, uncompressedSize, DisposeAfterUse::YES);

	// Read the contents of the uncompressed data
	MOVatom a = { MKID_BE('moov'), 0, uncompressedSize };
	int err = readDefault(a);

	// Assign the file handle back to the original handle
	free(compressedData);
	delete _fd;
	_fd = oldStream;

	return err;
#else
	warning ("zlib not found, cannot read QuickTime cmov atom");
	return -1;
#endif
}

int QuickTimeDecoder::readMVHD(MOVatom atom) {
	byte version = _fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	if (version == 1) {
		warning("QuickTime version 1");
		_fd->readUint32BE(); _fd->readUint32BE();
		_fd->readUint32BE(); _fd->readUint32BE();
	} else {
		_fd->readUint32BE(); // creation time
		_fd->readUint32BE(); // modification time
	}

	_timeScale = _fd->readUint32BE(); // time scale
	debug(0, "time scale = %i\n", _timeScale);

	// duration
	_duration = (version == 1) ? (_fd->readUint32BE(), _fd->readUint32BE()) : _fd->readUint32BE();
	_fd->readUint32BE(); // preferred scale

	_fd->readUint16BE(); // preferred volume

	_fd->seek(10, SEEK_CUR); // reserved

	// We only need two values from the movie display matrix. Most of the values are just
	// skipped. xMod and yMod are 16:16 fixed point numbers, the last part of the 3x3 matrix
	// is 2:30.
	uint32 xMod = _fd->readUint32BE();
	_fd->skip(12);
	uint32 yMod = _fd->readUint32BE();
	_fd->skip(16);

	_scaleFactorX = Common::Rational(0x10000, xMod);
	_scaleFactorY = Common::Rational(0x10000, yMod);

	_scaleFactorX.debugPrint(1, "readMVHD(): scaleFactorX =");
	_scaleFactorY.debugPrint(1, "readMVHD(): scaleFactorY =");

	_fd->readUint32BE(); // preview time
	_fd->readUint32BE(); // preview duration
	_fd->readUint32BE(); // poster time
	_fd->readUint32BE(); // selection time
	_fd->readUint32BE(); // selection duration
	_fd->readUint32BE(); // current time
	_fd->readUint32BE(); // next track ID

	return 0;
}

int QuickTimeDecoder::readTRAK(MOVatom atom) {
	MOVStreamContext *sc = new MOVStreamContext();

	if (!sc)
		return -1;

	sc->sample_to_chunk_index = -1;
	sc->codec_type = CODEC_TYPE_MOV_OTHER;
	sc->start_time = 0; // XXX: check
	_streams[_numStreams++] = sc;

	return readDefault(atom);
}

// this atom contains actual media data
int QuickTimeDecoder::readMDAT(MOVatom atom) {
	if (atom.size == 0) // wrong one (MP4)
		return 0;

	_foundMDAT = true;

	_mdatOffset = atom.offset;
	_mdatSize = atom.size;

	if (_foundMOOV)
		return 1; // found both, just go

	_fd->seek(atom.size, SEEK_CUR);

	return 0; // now go for moov
}

int QuickTimeDecoder::readTKHD(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];
	byte version = _fd->readByte();

	_fd->readByte(); _fd->readByte();
	_fd->readByte(); // flags
	//
	//MOV_TRACK_ENABLED 0x0001
	//MOV_TRACK_IN_MOVIE 0x0002
	//MOV_TRACK_IN_PREVIEW 0x0004
	//MOV_TRACK_IN_POSTER 0x0008
	//

	if (version == 1) {
		_fd->readUint32BE(); _fd->readUint32BE();
		_fd->readUint32BE(); _fd->readUint32BE();
	} else {
		_fd->readUint32BE(); // creation time
		_fd->readUint32BE(); // modification time
	}

	/* st->id = */_fd->readUint32BE(); // track id (NOT 0 !)
	_fd->readUint32BE(); // reserved
	//st->start_time = 0; // check
	(version == 1) ? (_fd->readUint32BE(), _fd->readUint32BE()) : _fd->readUint32BE(); // highlevel (considering edits) duration in movie timebase
	_fd->readUint32BE(); // reserved
	_fd->readUint32BE(); // reserved

	_fd->readUint16BE(); // layer
	_fd->readUint16BE(); // alternate group
	_fd->readUint16BE(); // volume
	_fd->readUint16BE(); // reserved

	// We only need the two values from the displacement matrix for a track.
	// See readMVHD() for more information.
	uint32 xMod = _fd->readUint32BE();
	_fd->skip(12);
	uint32 yMod = _fd->readUint32BE();
	_fd->skip(16);

	st->scaleFactorX = Common::Rational(0x10000, xMod);
	st->scaleFactorY = Common::Rational(0x10000, yMod);

	st->scaleFactorX.debugPrint(1, "readTKHD(): scaleFactorX =");
	st->scaleFactorY.debugPrint(1, "readTKHD(): scaleFactorY =");

	// these are fixed-point, 16:16
	// uint32 tkWidth = _fd->readUint32BE() >> 16; // track width
	// uint32 tkHeight = _fd->readUint32BE() >> 16; // track height

	return 0;
}

// edit list atom
int QuickTimeDecoder::readELST(MOVatom atom) {
	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags
	uint32 editCount = _streams[_numStreams - 1]->edit_count = _fd->readUint32BE();	 // entries

	for (uint32 i = 0; i < editCount; i++){
		_fd->readUint32BE(); // Track duration
		_fd->readUint32BE(); // Media time
		_fd->readUint32BE(); // Media rate
	}

	debug(0, "track[%i].edit_count = %i", _numStreams - 1, _streams[_numStreams - 1]->edit_count);

	if (editCount != 1)
		warning("Multiple edit list entries. Things may go awry");

	return 0;
}

int QuickTimeDecoder::readHDLR(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	// component type
	uint32 ctype = _fd->readUint32LE();
	uint32 type = _fd->readUint32BE(); // component subtype

	debug(0, "ctype= %s (0x%08lx)", tag2str(ctype), (long)ctype);
	debug(0, "stype= %s", tag2str(type));

	if(ctype == MKID_BE('mhlr')) // MOV
		debug(0, "MOV detected");
	else if(ctype == 0) {
		warning("MP4 streams are not supported");
		return -1;
	}

	if (type == MKID_BE('vide'))
		st->codec_type = CODEC_TYPE_VIDEO;
	else if (type == MKID_BE('soun'))
		st->codec_type = CODEC_TYPE_AUDIO;

	_fd->readUint32BE(); // component  manufacture
	_fd->readUint32BE(); // component flags
	_fd->readUint32BE(); // component flags mask

	if (atom.size <= 24)
		return 0; // nothing left to read

	// .mov: PASCAL string
	byte len = _fd->readByte();
	_fd->seek(len, SEEK_CUR);

	_fd->seek(atom.size - (_fd->pos() - atom.offset), SEEK_CUR);

	return 0;
}

int QuickTimeDecoder::readMDHD(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];
	byte version = _fd->readByte();

	if (version > 1)
		return 1; // unsupported

	_fd->readByte(); _fd->readByte();
	_fd->readByte(); // flags

	if (version == 1) {
		_fd->readUint32BE(); _fd->readUint32BE();
		_fd->readUint32BE(); _fd->readUint32BE();
	} else {
		_fd->readUint32BE(); // creation time
		_fd->readUint32BE(); // modification time
	}

	st->time_scale = _fd->readUint32BE();
	st->duration = (version == 1) ? (_fd->readUint32BE(), _fd->readUint32BE()) : _fd->readUint32BE(); // duration

	_fd->readUint16BE(); // language
	_fd->readUint16BE(); // quality

	return 0;
}

int QuickTimeDecoder::readSTSD(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	uint32 entries = _fd->readUint32BE();

	while (entries--) { //Parsing Sample description table
		MOVatom a = { 0, 0, 0 };
		uint32 start_pos = _fd->pos();
		int size = _fd->readUint32BE(); // size
		uint32 format = _fd->readUint32BE(); // data format

		_fd->readUint32BE(); // reserved
		_fd->readUint16BE(); // reserved
		_fd->readUint16BE(); // index

		debug(0, "size=%d 4CC= %s codec_type=%d", size, tag2str(format), st->codec_type);

		if (st->codec_tag && st->codec_tag != format) {
			// HACK: Multiple FourCC, skip this. FFmpeg does this too and also
			// skips it with a TODO. However, we really don't need to support
			// multiple codec tags since the only two videos in Riven DVD that
			// do this just have a fake second stream (or so it seems).
			debug(3, "Multiple FourCC not supported");
			_fd->seek(start_pos + size);
			continue;
		}

		st->codec_tag = format;

		if (st->codec_type == CODEC_TYPE_VIDEO) {
			debug(0, "Video Codec FourCC: \'%s\'", tag2str(format));

			_fd->readUint16BE(); // version
			_fd->readUint16BE(); // revision level
			_fd->readUint32BE(); // vendor
			_fd->readUint32BE(); // temporal quality
			_fd->readUint32BE(); // spacial quality

			st->width = _fd->readUint16BE(); // width
			st->height = _fd->readUint16BE(); // height

			_fd->readUint32BE(); // horiz resolution
			_fd->readUint32BE(); // vert resolution
			_fd->readUint32BE(); // data size, always 0
			uint16 frames_per_sample = _fd->readUint16BE(); // frames per samples

			debug(0, "frames/samples = %d", frames_per_sample);

			byte codec_name[32];
			_fd->read(codec_name, 32); // codec name, pascal string (FIXME: true for mp4?)
			if (codec_name[0] <= 31) {
				memcpy(st->codec_name, &codec_name[1], codec_name[0]);
				st->codec_name[codec_name[0]] = 0;
			}

			st->bits_per_sample = _fd->readUint16BE(); // depth
			st->color_table_id = _fd->readUint16BE(); // colortable id

//		  These are set in mov_read_stts and might already be set!
//			st->codec->time_base.den	  = 25;
//			st->codec->time_base.num = 1;


			// figure out the palette situation
			byte colorDepth = st->bits_per_sample & 0x1F;
			bool colorGreyscale = (st->bits_per_sample & 0x20) != 0;

			debug(0, "color depth: %d", colorDepth);

			// if the depth is 2, 4, or 8 bpp, file is palettized
			if (colorDepth == 2 || colorDepth == 4 || colorDepth == 8) {
				_dirtyPalette = true;

				if (colorGreyscale) {
					debug(0, "Greyscale palette");

					// compute the greyscale palette
					uint16 colorCount = 1 << colorDepth;
					int16 colorIndex = 255;
					byte colorDec = 256 / (colorCount - 1);
					for (byte j = 0; j < colorCount; j++) {
						_palette[j * 3] = _palette[j * 3 + 1] = _palette[j * 3 + 2] = colorIndex;
						colorIndex -= colorDec;
						if (colorIndex < 0)
							colorIndex = 0;
					}
				} else if (st->color_table_id & 0x08) {
					// if flag bit 3 is set, use the default palette
					//uint16 colorCount = 1 << colorDepth;

					warning("Predefined palette! %dbpp", colorDepth);
#if 0
					byte *color_table;
					byte  r, g, b;

					if (colorDepth == 2)
						color_table = ff_qt_default_palette_4;
					else if (colorDepth == 4)
						color_table = ff_qt_default_palette_16;
					else
						color_table = ff_qt_default_palette_256;

					for (byte j = 0; j < color_count; j++) {
						r = color_table[j * 4 + 0];
						g = color_table[j * 4 + 1];
						b = color_table[j * 4 + 2];
						_palette_control.palette[j] = (r << 16) | (g << 8) | (b);
					}
#endif

				} else {
					debug(0, "Palette from file");

					// load the palette from the file
					uint32 colorStart = _fd->readUint32BE();
					/* uint16 colorCount = */ _fd->readUint16BE();
					uint16 colorEnd = _fd->readUint16BE();
					for (uint32 j = colorStart; j <= colorEnd; j++) {
						// each R, G, or B component is 16 bits;
						// only use the top 8 bits; skip alpha bytes
						// up front
						_fd->readByte();
						_fd->readByte();
						_palette[j * 3] = _fd->readByte();
						_fd->readByte();
						_palette[j * 3 + 1] = _fd->readByte();
						_fd->readByte();
						_palette[j * 3 + 2] = _fd->readByte();
						_fd->readByte();
					}
				}
				st->palettized = true;
			} else
				st->palettized = false;
		} else if (st->codec_type == CODEC_TYPE_AUDIO) {
			debug(0, "Audio Codec FourCC: \'%s\'", tag2str(format));

			st->stsd_version = _fd->readUint16BE();
			_fd->readUint16BE(); // revision level
			_fd->readUint32BE(); // vendor

			st->channels = _fd->readUint16BE();			 // channel count
			st->bits_per_sample = _fd->readUint16BE();	  // sample size
			// do we need to force to 16 for AMR ?

			// handle specific s8 codec
			_fd->readUint16BE(); // compression id = 0
			_fd->readUint16BE(); // packet size = 0

			st->sample_rate = (_fd->readUint32BE() >> 16);

			debug(0, "stsd version =%d", st->stsd_version);
			if (st->stsd_version == 0) {
				// Not used, except in special cases. See below.
				st->samples_per_frame = st->bytes_per_frame = 0;
			} else if (st->stsd_version == 1) {
				// Read QT version 1 fields. In version 0 these dont exist.
				st->samples_per_frame = _fd->readUint32BE();
				debug(0, "stsd samples_per_frame =%d", st->samples_per_frame);
				_fd->readUint32BE(); // bytes per packet
				st->bytes_per_frame = _fd->readUint32BE();
				debug(0, "stsd bytes_per_frame =%d", st->bytes_per_frame);
				_fd->readUint32BE(); // bytes per sample
			} else {
				warning("Unsupported QuickTime STSD audio version %d", st->stsd_version);
				return 1;
			}

			// Version 0 videos (such as the Riven ones) don't have this set,
			// but we need it later on. Add it in here.
			if (format == MKID_BE('ima4')) {
				st->samples_per_frame = 64;
				st->bytes_per_frame = 34 * st->channels;
			}
		} else {
			// other codec type, just skip (rtp, mp4s, tmcd ...)
			_fd->seek(size - (_fd->pos() - start_pos), SEEK_CUR);
		}

		// this will read extra atoms at the end (wave, alac, damr, avcC, SMI ...)
		a.size = size - (_fd->pos() - start_pos);
		if (a.size > 8)
			readDefault(a);
		else if (a.size > 0)
			_fd->seek(a.size, SEEK_CUR);
	}

	if (st->codec_type == CODEC_TYPE_AUDIO && st->sample_rate == 0 && st->time_scale > 1)
		st->sample_rate= st->time_scale;

	return 0;
}

int QuickTimeDecoder::readSTSC(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->sample_to_chunk_sz = _fd->readUint32BE();

	debug(0, "track[%i].stsc.entries = %i", _numStreams - 1, st->sample_to_chunk_sz);

	st->sample_to_chunk = new MOVstsc[st->sample_to_chunk_sz];

	if (!st->sample_to_chunk)
		return -1;

	for (uint32 i = 0; i < st->sample_to_chunk_sz; i++) {
		st->sample_to_chunk[i].first = _fd->readUint32BE();
		st->sample_to_chunk[i].count = _fd->readUint32BE();
		st->sample_to_chunk[i].id = _fd->readUint32BE();
		//printf ("Sample to Chunk[%d]: First = %d, Count = %d\n", i, st->sample_to_chunk[i].first, st->sample_to_chunk[i].count);
	}

	return 0;
}

int QuickTimeDecoder::readSTSS(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->keyframe_count = _fd->readUint32BE();

	debug(0, "keyframe_count = %d", st->keyframe_count);

	st->keyframes = new uint32[st->keyframe_count];

	if (!st->keyframes)
		return -1;

	for (uint32 i = 0; i < st->keyframe_count; i++) {
		st->keyframes[i] = _fd->readUint32BE();
		debug(6, "keyframes[%d] = %d", i, st->keyframes[i]);

	}
	return 0;
}

int QuickTimeDecoder::readSTSZ(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->sample_size = _fd->readUint32BE();
	st->sample_count = _fd->readUint32BE();

	debug(5, "sample_size = %d sample_count = %d", st->sample_size, st->sample_count);

	if (st->sample_size)
		return 0; // there isn't any table following

	st->sample_sizes = new uint32[st->sample_count];

	if (!st->sample_sizes)
		return -1;

	for(uint32 i = 0; i < st->sample_count; i++) {
		st->sample_sizes[i] = _fd->readUint32BE();
		debug(6, "sample_sizes[%d] = %d", i, st->sample_sizes[i]);
	}

	return 0;
}

static uint32 ff_gcd(uint32 a, uint32 b) {
	if(b) return ff_gcd(b, a%b);
	else  return a;
}

int QuickTimeDecoder::readSTTS(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];
	uint32 duration = 0;
	uint32 total_sample_count = 0;

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->stts_count = _fd->readUint32BE();
	st->stts_data = new MOVstts[st->stts_count];

	debug(0, "track[%i].stts.entries = %i", _numStreams - 1, st->stts_count);

	st->time_rate = 0;

	for (int32 i = 0; i < st->stts_count; i++) {
		int sample_duration;
		int sample_count;

		sample_count = _fd->readUint32BE();
		sample_duration = _fd->readUint32BE();
		st->stts_data[i].count = sample_count;
		st->stts_data[i].duration = sample_duration;

		st->time_rate = ff_gcd(st->time_rate, sample_duration);

		debug(0, "sample_count=%d, sample_duration=%d", sample_count, sample_duration);

		duration += sample_duration * sample_count;
		total_sample_count += sample_count;
	}

	st->nb_frames = total_sample_count;

	if (duration)
		st->duration = duration;

	return 0;
}

int QuickTimeDecoder::readSTCO(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->chunk_count = _fd->readUint32BE();
	st->chunk_offsets = new uint32[st->chunk_count];

	if (!st->chunk_offsets)
		return -1;

	for (uint32 i = 0; i < st->chunk_count; i++) {
		// WORKAROUND/HACK: The offsets in Riven videos (ones inside the Mohawk archives themselves)
		// have offsets relative to the archive and not the video. This is quite nasty. We subtract
		// the initial offset of the stream to get the correct value inside of the stream.
		st->chunk_offsets[i] = _fd->readUint32BE() - _beginOffset;
	}

	for (uint32 i = 0; i < _numStreams; i++) {
		MOVStreamContext *sc2 = _streams[i];

		if(sc2 && sc2->chunk_offsets){
			uint32 first = sc2->chunk_offsets[0];
			uint32 last = sc2->chunk_offsets[sc2->chunk_count - 1];

			if(first >= st->chunk_offsets[st->chunk_count - 1] || last <= st->chunk_offsets[0])
				_ni = 1;
		}
	}

	return 0;
}

int QuickTimeDecoder::readWAVE(MOVatom atom) {
	if (_numStreams < 1)
		return 0;

	MOVStreamContext *st = _streams[_numStreams - 1];

	if (atom.size > (1 << 30))
		return -1;

	if (st->codec_tag == MKID_BE('QDM2')) // Read extradata for QDM2
		st->extradata = _fd->readStream(atom.size - 8);
	else if (atom.size > 8)
		return readDefault(atom);
	else
		_fd->skip(atom.size);

	return 0;
}

void QuickTimeDecoder::close() {
	stopAudio();

	delete _videoCodec; _videoCodec = 0;

	for (uint32 i = 0; i < _numStreams; i++)
		delete _streams[i];

	delete _fd;
	_fd = 0;

	if (_scaledSurface) {
		_scaledSurface->free();
		delete _scaledSurface;
		_scaledSurface = 0;
	}

	// The audio stream is deleted automatically
	_audStream = NULL;

	VideoDecoder::reset();
}

Common::SeekableReadStream *QuickTimeDecoder::getNextFramePacket() {
	if (_videoStreamIndex < 0)
		return NULL;

	// First, we have to track down which chunk holds the sample and which sample in the chunk contains the frame we are looking for.
	int32 totalSampleCount = 0;
	int32 sampleInChunk = 0;
	int32 actualChunk = -1;

	for (uint32 i = 0; i < _streams[_videoStreamIndex]->chunk_count; i++) {
		int32 sampleToChunkIndex = -1;

		for (uint32 j = 0; j < _streams[_videoStreamIndex]->sample_to_chunk_sz; j++)
			if (i >= _streams[_videoStreamIndex]->sample_to_chunk[j].first - 1)
				sampleToChunkIndex = j;

		if (sampleToChunkIndex < 0)
			error("This chunk (%d) is imaginary", sampleToChunkIndex);

		totalSampleCount += _streams[_videoStreamIndex]->sample_to_chunk[sampleToChunkIndex].count;

		if (totalSampleCount > getCurFrame()) {
			actualChunk = i;
			sampleInChunk = _streams[_videoStreamIndex]->sample_to_chunk[sampleToChunkIndex].count - totalSampleCount + getCurFrame();
			break;
		}
	}

	if (actualChunk < 0) {
		warning ("Could not find data for frame %d", getCurFrame());
		return NULL;
	}

	// Next seek to that frame
	_fd->seek(_streams[_videoStreamIndex]->chunk_offsets[actualChunk]);

	// Then, if the chunk holds more than one frame, seek to where the frame we want is located
	for (int32 i = getCurFrame() - sampleInChunk; i < getCurFrame(); i++) {
		if (_streams[_videoStreamIndex]->sample_size != 0)
			_fd->skip(_streams[_videoStreamIndex]->sample_size);
		else
			_fd->skip(_streams[_videoStreamIndex]->sample_sizes[i]);
	}

	// Finally, read in the raw data for the frame
	//printf ("Frame Data[%d]: Offset = %d, Size = %d\n", getCurFrame(), _fd->pos(), _streams[_videoStreamIndex]->sample_sizes[getCurFrame()]);

	if (_streams[_videoStreamIndex]->sample_size != 0)
		return _fd->readStream(_streams[_videoStreamIndex]->sample_size);

	return _fd->readStream(_streams[_videoStreamIndex]->sample_sizes[getCurFrame()]);
}

bool QuickTimeDecoder::checkAudioCodecSupport(uint32 tag) {
	// Check if the codec is a supported codec
	if (tag == MKID_BE('twos') || tag == MKID_BE('raw ') || tag == MKID_BE('ima4'))
		return true;

#ifdef GRAPHICS_QDM2_H
	if (tag == MKID_BE('QDM2'))
		return true;
#endif

	warning("Audio Codec Not Supported: \'%s\'", tag2str(tag));

	return false;
}

Audio::AudioStream *QuickTimeDecoder::createAudioStream(Common::SeekableReadStream *stream) {
	if (!stream || _audioStreamIndex < 0)
		return NULL;

	if (_streams[_audioStreamIndex]->codec_tag == MKID_BE('twos') || _streams[_audioStreamIndex]->codec_tag == MKID_BE('raw ')) {
		// Fortunately, most of the audio used in Myst videos is raw...
		uint16 flags = 0;
		if (_streams[_audioStreamIndex]->codec_tag == MKID_BE('raw '))
			flags |= Audio::FLAG_UNSIGNED;
		if (_streams[_audioStreamIndex]->channels == 2)
			flags |= Audio::FLAG_STEREO;
		if (_streams[_audioStreamIndex]->bits_per_sample == 16)
			flags |= Audio::FLAG_16BITS;
		uint32 dataSize = stream->size();
		byte *data = (byte *)malloc(dataSize);
		stream->read(data, dataSize);
		delete stream;
		return Audio::makeRawStream(data, dataSize, _streams[_audioStreamIndex]->sample_rate, flags);
	} else if (_streams[_audioStreamIndex]->codec_tag == MKID_BE('ima4')) {
		// Riven uses this codec (as do some Myst ME videos)
		return Audio::makeADPCMStream(stream, DisposeAfterUse::YES, stream->size(), Audio::kADPCMApple, _streams[_audioStreamIndex]->sample_rate, _streams[_audioStreamIndex]->channels, 34);
#ifdef GRAPHICS_QDM2_H
	} else if (_streams[_audioStreamIndex]->codec_tag == MKID_BE('QDM2')) {
		// Several Myst ME videos use this codec
		return makeQDM2Stream(stream, _streams[_audioStreamIndex]->extradata);
#endif
	}

	error("Unsupported audio codec");

	return NULL;
}

void QuickTimeDecoder::updateAudioBuffer() {
	if (!_audStream)
		return;

	// Keep three streams in buffer so that if/when the first two end, it goes right into the next
	for (; _audStream->numQueuedStreams() < 3 && _curAudioChunk < _streams[_audioStreamIndex]->chunk_count; _curAudioChunk++) {
		Common::MemoryWriteStreamDynamic *wStream = new Common::MemoryWriteStreamDynamic();

		_fd->seek(_streams[_audioStreamIndex]->chunk_offsets[_curAudioChunk]);

		// First, we have to get the sample count
		uint32 sampleCount = 0;
		for (uint32 j = 0; j < _streams[_audioStreamIndex]->sample_to_chunk_sz; j++)
			if (_curAudioChunk >= (_streams[_audioStreamIndex]->sample_to_chunk[j].first - 1))
				sampleCount = _streams[_audioStreamIndex]->sample_to_chunk[j].count;
		assert(sampleCount);

		// Then calculate the right sizes
		while (sampleCount > 0) {
			uint32 samples = 0, size = 0;

			if (_streams[_audioStreamIndex]->samples_per_frame >= 160) {
				samples = _streams[_audioStreamIndex]->samples_per_frame;
				size = _streams[_audioStreamIndex]->bytes_per_frame;
			} else if (_streams[_audioStreamIndex]->samples_per_frame > 1) {
				samples = MIN<uint32>((1024 / _streams[_audioStreamIndex]->samples_per_frame) * _streams[_audioStreamIndex]->samples_per_frame, sampleCount);
				size = (samples / _streams[_audioStreamIndex]->samples_per_frame) * _streams[_audioStreamIndex]->bytes_per_frame;
			} else {
				samples = MIN<uint32>(1024, sampleCount);
				size = samples * _streams[_audioStreamIndex]->sample_size;
			}

			// Now, we read in the data for this data and output it
			byte *data = (byte *)malloc(size);
			_fd->read(data, size);
			wStream->write(data, size);
			free(data);
			sampleCount -= samples;
		}

		// Now queue the buffer
		_audStream->queueAudioStream(createAudioStream(new Common::MemoryReadStream(wStream->getData(), wStream->size(), DisposeAfterUse::YES)));
		delete wStream;
	}
}

} // End of namespace Graphics
