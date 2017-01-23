/**
* @file Audio.cpp
*/
#include "Audio.h"
#include <xaudio2.h>
#include <vector>
#include <list>
#include <stdint.h>
#include <wrl/client.h>
#include <algorithm>

#pragma comment(lib, "xaudio2.lib")

using Microsoft::WRL::ComPtr;

namespace Audio {

typedef std::vector<uint8_t> BufferType;

struct ScopedHandle
{
	ScopedHandle(HANDLE h) : handle(h == INVALID_HANDLE_VALUE ? 0 : h) {}
	~ScopedHandle() { if (handle) { CloseHandle(handle); } }
	operator HANDLE() { return handle; }
	HANDLE handle;
};

/**
* WAVデータ.
*/
struct WavData
{
	const WAVEFORMATEX* wfx;
	const uint8_t* startAudio;
	uint32_t audioBytes;
	uint32_t loopStart;
	uint32_t loopLength;
	const uint32_t* seek;
	uint32_t seekCount;
};

const uint32_t FOURCC_RIFF_TAG = 'FFIR';
const uint32_t FOURCC_FORMAT_TAG = ' tmf';
const uint32_t FOURCC_DATA_TAG = 'atad';
const uint32_t FOURCC_WAVE_FILE_TAG = 'EVAW';
const uint32_t FOURCC_XWMA_FILE_TAG = 'AMWX';
const uint32_t FOURCC_DLS_SAMPLE = 'pmsw';
const uint32_t FOURCC_MIDI_SAMPLE = 'lpms';
const uint32_t FOURCC_XWMA_DPDS = 'sdpd';
const uint32_t FOURCC_XMA_SEEK = 'kees';

#pragma pack(push,1)
struct RIFFChunk
{
	uint32_t tag;
	uint32_t size;
};

struct RIFFChunkHeader
{
	uint32_t tag;
	uint32_t size;
	uint32_t riff;
};

struct DLSLoop
{
	static const uint32_t LOOP_TYPE_FORWARD = 0x00000000;
	static const uint32_t LOOP_TYPE_RELEASE = 0x00000001;

	uint32_t size;
	uint32_t loopType;
	uint32_t loopStart;
	uint32_t loopLength;
};

struct RIFFDLSSample
{
	static const uint32_t OPTIONS_NOTRUNCATION = 0x00000001;
	static const uint32_t OPTIONS_NOCOMPRESSION = 0x00000002;

	uint32_t    size;
	uint16_t    unityNote;
	int16_t     fineTune;
	int32_t     gain;
	uint32_t    options;
	uint32_t    loopCount;
};

struct MIDILoop
{
	static const uint32_t LOOP_TYPE_FORWARD = 0x00000000;
	static const uint32_t LOOP_TYPE_ALTERNATING = 0x00000001;
	static const uint32_t LOOP_TYPE_BACKWARD = 0x00000002;

	uint32_t cuePointId;
	uint32_t type;
	uint32_t start;
	uint32_t end;
	uint32_t fraction;
	uint32_t playCount;
};

struct RIFFMIDISample
{
	uint32_t        manufacturerId;
	uint32_t        productId;
	uint32_t        samplePeriod;
	uint32_t        unityNode;
	uint32_t        pitchFraction;
	uint32_t        SMPTEFormat;
	uint32_t        SMPTEOffset;
	uint32_t        loopCount;
	uint32_t        samplerData;
};
#pragma pack(pop)

static const RIFFChunk* FindChunk(const uint8_t* data, size_t sizeBytes, uint32_t tag)
{
	if (!data)
		return nullptr;

	const uint8_t* ptr = data;
	const uint8_t* end = data + sizeBytes;

	while (end > (ptr + sizeof(RIFFChunk))) {
		auto header = reinterpret_cast<const RIFFChunk*>(ptr);
		if (header->tag == tag) {
			return header;
		}
		ptrdiff_t offset = header->size + sizeof(RIFFChunk);
		ptr += offset;
	}

	return nullptr;
}

static HRESULT WaveFindFormatAndData(
	const uint8_t* wavData,
	size_t wavDataSize,
	const WAVEFORMATEX** pwfx,
	const uint8_t** pdata,
	uint32_t* dataSize,
	bool& dpds, bool& seek)
{
	if (!wavData || !pwfx)
		return E_POINTER;

	dpds = seek = false;

	if (wavDataSize < (sizeof(RIFFChunk) * 2 + sizeof(uint32_t) + sizeof(WAVEFORMAT)))
	{
		return E_FAIL;
	}

	const uint8_t* wavEnd = wavData + wavDataSize;

	// Locate RIFF 'WAVE'
	auto riffChunk = FindChunk(wavData, wavDataSize, FOURCC_RIFF_TAG);
	if (!riffChunk || riffChunk->size < 4)
	{
		return E_FAIL;
	}

	auto riffHeader = reinterpret_cast<const RIFFChunkHeader*>(riffChunk);
	if (riffHeader->riff != FOURCC_WAVE_FILE_TAG && riffHeader->riff != FOURCC_XWMA_FILE_TAG)
	{
		return E_FAIL;
	}

	// Locate 'fmt '
	auto ptr = reinterpret_cast<const uint8_t*>(riffHeader) + sizeof(RIFFChunkHeader);
	if ((ptr + sizeof(RIFFChunk)) > wavEnd)
	{
		return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
	}

	auto fmtChunk = FindChunk(ptr, riffHeader->size, FOURCC_FORMAT_TAG);
	if (!fmtChunk || fmtChunk->size < sizeof(PCMWAVEFORMAT))
	{
		return E_FAIL;
	}

	ptr = reinterpret_cast<const uint8_t*>(fmtChunk) + sizeof(RIFFChunk);
	if (ptr + fmtChunk->size > wavEnd)
	{
		return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
	}

	auto wf = reinterpret_cast<const WAVEFORMAT*>(ptr);

	// Validate WAVEFORMAT (focused on chunk size and format tag, not other data that XAUDIO2 will validate)
	switch (wf->wFormatTag)
	{
	case WAVE_FORMAT_PCM:
	case WAVE_FORMAT_IEEE_FLOAT:
		// Can be a PCMWAVEFORMAT (8 bytes) or WAVEFORMATEX (10 bytes)
		// We validiated chunk as at least sizeof(PCMWAVEFORMAT) above
		break;

	default:
	{
		if (fmtChunk->size < sizeof(WAVEFORMATEX))
		{
			return E_FAIL;
		}

		auto wfx = reinterpret_cast<const WAVEFORMATEX*>(ptr);

		if (fmtChunk->size < (sizeof(WAVEFORMATEX) + wfx->cbSize))
		{
			return E_FAIL;
		}

		switch (wfx->wFormatTag)
		{
		case WAVE_FORMAT_WMAUDIO2:
		case WAVE_FORMAT_WMAUDIO3:
			dpds = true;
			break;

		case WAVE_FORMAT_ADPCM:
			if ((fmtChunk->size < (sizeof(WAVEFORMATEX) + 32)) || (wfx->cbSize < 32 /*MSADPCM_FORMAT_EXTRA_BYTES*/))
			{
				return E_FAIL;
			}
			break;

		case WAVE_FORMAT_EXTENSIBLE:
			if ((fmtChunk->size < sizeof(WAVEFORMATEXTENSIBLE)) || (wfx->cbSize < (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))))
			{
				return E_FAIL;
			} else
			{
				static const GUID s_wfexBase = { 0x00000000, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 };

				auto wfex = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(ptr);

				if (memcmp(reinterpret_cast<const BYTE*>(&wfex->SubFormat) + sizeof(DWORD),
					reinterpret_cast<const BYTE*>(&s_wfexBase) + sizeof(DWORD), sizeof(GUID) - sizeof(DWORD)) != 0)
				{
					return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
				}

				switch (wfex->SubFormat.Data1)
				{
				case WAVE_FORMAT_PCM:
				case WAVE_FORMAT_IEEE_FLOAT:
					break;

					// MS-ADPCM and XMA2 are not supported as WAVEFORMATEXTENSIBLE

				case WAVE_FORMAT_WMAUDIO2:
				case WAVE_FORMAT_WMAUDIO3:
					dpds = true;
					break;

				default:
					return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
				}

			}
			break;

		default:
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}
	}
	}

	// Locate 'data'
	ptr = reinterpret_cast<const uint8_t*>(riffHeader) + sizeof(RIFFChunkHeader);
	if ((ptr + sizeof(RIFFChunk)) > wavEnd)
	{
		return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
	}

	auto dataChunk = FindChunk(ptr, riffChunk->size, FOURCC_DATA_TAG);
	if (!dataChunk || !dataChunk->size)
	{
		return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}

	ptr = reinterpret_cast<const uint8_t*>(dataChunk) + sizeof(RIFFChunk);
	if (ptr + dataChunk->size > wavEnd)
	{
		return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
	}

	*pwfx = reinterpret_cast<const WAVEFORMATEX*>(wf);
	*pdata = ptr;
	*dataSize = dataChunk->size;
	return S_OK;
}


static HRESULT WaveFindLoopInfo(_In_reads_bytes_(wavDataSize) const uint8_t* wavData, _In_ size_t wavDataSize,
	_Out_ uint32_t* pLoopStart, _Out_ uint32_t* pLoopLength)
{
	if (!wavData || !pLoopStart || !pLoopLength)
		return E_POINTER;

	if (wavDataSize < (sizeof(RIFFChunk) + sizeof(uint32_t)))
	{
		return E_FAIL;
	}

	*pLoopStart = 0;
	*pLoopLength = 0;

	const uint8_t* wavEnd = wavData + wavDataSize;

	// Locate RIFF 'WAVE'
	auto riffChunk = FindChunk(wavData, wavDataSize, FOURCC_RIFF_TAG);
	if (!riffChunk || riffChunk->size < 4)
	{
		return E_FAIL;
	}

	auto riffHeader = reinterpret_cast<const RIFFChunkHeader*>(riffChunk);
	if (riffHeader->riff == FOURCC_XWMA_FILE_TAG)
	{
		// xWMA files do not contain loop information
		return S_OK;
	}

	if (riffHeader->riff != FOURCC_WAVE_FILE_TAG)
	{
		return E_FAIL;
	}

	// Locate 'wsmp' (DLS Chunk)
	auto ptr = reinterpret_cast<const uint8_t*>(riffHeader) + sizeof(RIFFChunkHeader);
	if ((ptr + sizeof(RIFFChunk)) > wavEnd)
	{
		return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
	}

	auto dlsChunk = FindChunk(ptr, riffChunk->size, FOURCC_DLS_SAMPLE);
	if (dlsChunk)
	{
		ptr = reinterpret_cast<const uint8_t*>(dlsChunk) + sizeof(RIFFChunk);
		if (ptr + dlsChunk->size > wavEnd)
		{
			return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
		}

		if (dlsChunk->size >= sizeof(RIFFDLSSample))
		{
			auto dlsSample = reinterpret_cast<const RIFFDLSSample*>(ptr);

			if (dlsChunk->size >= (dlsSample->size + dlsSample->loopCount * sizeof(DLSLoop)))
			{
				auto loops = reinterpret_cast<const DLSLoop*>(ptr + dlsSample->size);
				for (uint32_t j = 0; j < dlsSample->loopCount; ++j)
				{
					if ((loops[j].loopType == DLSLoop::LOOP_TYPE_FORWARD || loops[j].loopType == DLSLoop::LOOP_TYPE_RELEASE))
					{
						// Return 'forward' loop
						*pLoopStart = loops[j].loopStart;
						*pLoopLength = loops[j].loopLength;
						return S_OK;
					}
				}
			}
		}
	}

	// Locate 'smpl' (Sample Chunk)
	auto midiChunk = FindChunk(ptr, riffChunk->size, FOURCC_MIDI_SAMPLE);
	if (midiChunk)
	{
		ptr = reinterpret_cast<const uint8_t*>(midiChunk) + sizeof(RIFFChunk);
		if (ptr + midiChunk->size > wavEnd)
		{
			return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
		}

		if (midiChunk->size >= sizeof(RIFFMIDISample))
		{
			auto midiSample = reinterpret_cast<const RIFFMIDISample*>(ptr);

			if (midiChunk->size >= (sizeof(RIFFMIDISample) + midiSample->loopCount * sizeof(MIDILoop)))
			{
				auto loops = reinterpret_cast<const MIDILoop*>(ptr + sizeof(RIFFMIDISample));
				for (uint32_t j = 0; j < midiSample->loopCount; ++j)
				{
					if (loops[j].type == MIDILoop::LOOP_TYPE_FORWARD)
					{
						// Return 'forward' loop
						*pLoopStart = loops[j].start;
						*pLoopLength = loops[j].end + loops[j].start + 1;
						return S_OK;
					}
				}
			}
		}
	}

	return S_OK;
}


static HRESULT WaveFindTable(_In_reads_bytes_(wavDataSize) const uint8_t* wavData, _In_ size_t wavDataSize, _In_ uint32_t tag,
	_Outptr_result_maybenull_ const uint32_t** pData, _Out_ uint32_t* dataCount)
{
	if (!wavData || !pData || !dataCount)
		return E_POINTER;

	if (wavDataSize < (sizeof(RIFFChunk) + sizeof(uint32_t)))
	{
		return E_FAIL;
	}

	*pData = nullptr;
	*dataCount = 0;

	const uint8_t* wavEnd = wavData + wavDataSize;

	// Locate RIFF 'WAVE'
	auto riffChunk = FindChunk(wavData, wavDataSize, FOURCC_RIFF_TAG);
	if (!riffChunk || riffChunk->size < 4)
	{
		return E_FAIL;
	}

	auto riffHeader = reinterpret_cast<const RIFFChunkHeader*>(riffChunk);
	if (riffHeader->riff != FOURCC_WAVE_FILE_TAG && riffHeader->riff != FOURCC_XWMA_FILE_TAG)
	{
		return E_FAIL;
	}

	// Locate tag
	auto ptr = reinterpret_cast<const uint8_t*>(riffHeader) + sizeof(RIFFChunkHeader);
	if ((ptr + sizeof(RIFFChunk)) > wavEnd)
	{
		return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
	}

	auto tableChunk = FindChunk(ptr, riffChunk->size, tag);
	if (tableChunk)
	{
		ptr = reinterpret_cast<const uint8_t*>(tableChunk) + sizeof(RIFFChunk);
		if (ptr + tableChunk->size > wavEnd)
		{
			return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
		}

		if ((tableChunk->size % sizeof(uint32_t)) != 0)
		{
			return E_FAIL;
		}

		*pData = reinterpret_cast<const uint32_t*>(ptr);
		*dataCount = tableChunk->size / 4;
	}

	return S_OK;
}

bool LoadWavDataFromFile(const wchar_t* filename, BufferType& buf, WavData& wd)
{
	wd = WavData{};
	ScopedHandle hFile = CreateFile2(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
	if (!hFile) {
		return false;
	}
	FILE_STANDARD_INFO fileInfo;
	if (!GetFileInformationByHandleEx(hFile, FileStandardInfo, &fileInfo, sizeof(fileInfo))) {
		return false;
	}
	if (fileInfo.EndOfFile.HighPart > 0) {
		return false;
	}
	if (fileInfo.EndOfFile.LowPart < (sizeof(RIFFChunk) * 2 + sizeof(DWORD) + sizeof(WAVEFORMAT))) {
		return false;
	}
	buf.resize(fileInfo.EndOfFile.LowPart);
	if (buf.size() < fileInfo.EndOfFile.LowPart) {
		return false;
	}
	DWORD bytesRead;
	if (!ReadFile(hFile, buf.data(), fileInfo.EndOfFile.LowPart, &bytesRead, nullptr)) {
		return false;
	}
	if (bytesRead < fileInfo.EndOfFile.LowPart) {
		return false;
	}

	bool dpds, seek;
	if (FAILED(WaveFindFormatAndData(buf.data(), bytesRead, &wd.wfx, &wd.startAudio, &wd.audioBytes, dpds, seek))) {
		return false;
	}

	if (FAILED(WaveFindLoopInfo(buf.data(), bytesRead, &wd.loopStart, &wd.loopLength))) {
		return false;
	}
	if (dpds) {
		if (FAILED(WaveFindTable(buf.data(), bytesRead, FOURCC_XWMA_DPDS, &wd.seek, &wd.seekCount))) {
			return false;
		}
	} else if (seek) {
		if (FAILED(WaveFindTable(buf.data(), bytesRead, FOURCC_XMA_SEEK, &wd.seek, &wd.seekCount))) {
			return false;
		}
	}

	return true;
}


bool Read(HANDLE hFile, void* buf, DWORD size)
{
	DWORD readSize;
	if (!ReadFile(hFile, buf, size, &readSize, nullptr) || readSize != size) {
		return false;
	}
	return true;
}

struct WF
{
	WAVEFORMATEX waveFormat;
	size_t dataOffset;
	size_t dataSize;
	size_t seekOffset;
	size_t seekSize;
};

// フォーマット情報を取得
bool LoadWaveFile(const wchar_t* filename, WAVEFORMATEX* wfx, BufferType* data)
{
	ScopedHandle hFile = CreateFile2(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
	if (!hFile) {
		return false;
	}

	RIFFChunk riffChunk;
	if (!Read(hFile, &riffChunk, sizeof(riffChunk))) {
		return false;
	}
	if (riffChunk.tag != FOURCC_RIFF_TAG) {
		return false;
	}

	uint32_t fourcc;
	if (!Read(hFile, &fourcc, sizeof(fourcc))) {
		return false;
	}
	if (fourcc != FOURCC_WAVE_FILE_TAG) {
		return false;
	}

	bool hasWaveFormat = false;
	bool hasData = false;
	size_t offset = 12;
	do {
		if (SetFilePointer(hFile, offset, nullptr, FILE_BEGIN) != offset) {
			break;
		}

		RIFFChunk chunk;
		if (!Read(hFile, &chunk, sizeof(chunk))) {
			break;
		}

		if (chunk.tag  == FOURCC_FORMAT_TAG) {
			if (!Read(hFile, wfx, std::min(chunk.size, sizeof(WAVEFORMATEX)))) {
				break;
			}
			if (wfx->wFormatTag == WAVE_FORMAT_PCM) {
				wfx->cbSize = 0;
			}
			hasWaveFormat = true;
		} else if (chunk.tag == FOURCC_DATA_TAG) {
			data->resize(chunk.size);
			if (!Read(hFile, data->data(), chunk.size)) {
				return false;
			}
			hasData = true;
		}
		offset += chunk.size + sizeof(RIFFChunk);
	} while (!hasWaveFormat || !hasData);

	return hasWaveFormat && hasData;
}

// フォーマット情報を取得
bool GetWaveFormat(HANDLE hFile, WF& wf)
{
	bool hasWaveFormat = false;
	bool hasData = false;
	bool hasDpds = false;
	size_t offset = 12;
	do {
		SetFilePointer(hFile, offset, nullptr, FILE_BEGIN);

		RIFFChunk chunk;
		if (!Read(hFile, &chunk, sizeof(chunk))) {
			break;
		}

		if (chunk.tag == FOURCC_FORMAT_TAG) {
			if (!Read(hFile, &wf.waveFormat, std::min(chunk.size, sizeof(WAVEFORMATEX)))) {
				break;
			}
			if (wf.waveFormat.wFormatTag == WAVE_FORMAT_PCM) {
				wf.waveFormat.cbSize = 0;
				wf.seekSize = 0;
				wf.seekOffset = 0;
				hasDpds = true;
			}
			hasWaveFormat = true;
		} else if (chunk.tag == FOURCC_DATA_TAG) {
			wf.dataOffset = offset + sizeof(RIFFChunk);
			wf.dataSize = chunk.size;
			hasData = true;
		} else if (chunk.tag == FOURCC_XWMA_DPDS) {
			wf.seekOffset = offset + sizeof(RIFFChunk);
			wf.seekSize = chunk.size / 4;
			hasDpds = true;
		}
		offset += chunk.size + sizeof(RIFFChunk);
	} while (!hasWaveFormat || !hasData || !hasDpds);
	return hasWaveFormat && hasData && hasDpds;
}

/**
* Soundの実装.
*/
class SoundImpl : public Sound
{
public:
	SoundImpl() :
		started(false), paused(false), sourceVoice(nullptr) {}
	virtual ~SoundImpl() override {
		if (sourceVoice) {
			sourceVoice->DestroyVoice();
		}
	}
	virtual bool Play() override {
		if (started && !paused) {
			Stop();
		}
		started = true;
		paused = false;
		sourceVoice->SubmitSourceBuffer(&buffer);
		return SUCCEEDED(sourceVoice->Start());
	}
	virtual bool Pause() override {
		if (started) {
			started = false;
			return SUCCEEDED(sourceVoice->Stop());
		}
		return false;
	}
	virtual bool Seek() override {
		return true;
	}
	virtual bool Stop() override {
		if (started) {
			started = false;
			sourceVoice->Stop();
			sourceVoice->FlushSourceBuffers();
			return true;
		}
		return false;
	}
	virtual float SetVolume(float volume) override {
		sourceVoice->SetVolume(volume);
		return volume;
	}
	virtual float SetPitch(float pitch) override {
		sourceVoice->SetFrequencyRatio(pitch);
		return pitch;
	}
	virtual State GetState() const override {
		XAUDIO2_VOICE_STATE state;
		sourceVoice->GetState(&state);
		if (state.BuffersQueued) {
			return started ? State_Playing : State_Prepared;
		}
		return State_Stopped;
	}
	bool IsPlaying() const {
		return started && !paused;
	}

	bool started;
	bool paused;
	IXAudio2SourceVoice* sourceVoice;
	std::vector<uint8_t> file;
	XAUDIO2_BUFFER buffer;
};

/**
* Soundの実装.
*/
class StreamSoundImpl : public Sound
{
public:
	StreamSoundImpl() = delete;
	StreamSoundImpl(HANDLE h, size_t offset, size_t size) :
		started(false), sourceVoice(nullptr), handle(h), dataOffset(offset), dataSize(size)
	{
		buf.resize(BUFFER_SIZE * MAX_BUFFER_COUNT);
		currentPos = 0;
		seekInfo.PacketCount = 0;
	}
	virtual ~StreamSoundImpl() override {
		if (sourceVoice) {
			sourceVoice->DestroyVoice();
		}
	}
	virtual bool Play() override {
		if (!started) {
			started = true;
//			return SUCCEEDED(sourceVoice->Start());
		}
		return false;
	}
	virtual bool Pause() override {
		return SUCCEEDED(sourceVoice->Stop());
	}
	virtual bool Seek() override {
		return true;
	}
	virtual bool Stop() override {
		if (started) {
			started = false;
			return SUCCEEDED(sourceVoice->Stop());
		}
		return false;
	}
	virtual float SetVolume(float volume) override {
		sourceVoice->SetVolume(volume);
		return volume;
	}
	virtual float SetPitch(float pitch) override {
		sourceVoice->SetFrequencyRatio(pitch);
		return pitch;
	}
	virtual State GetState() const override {
		XAUDIO2_VOICE_STATE state;
		sourceVoice->GetState(&state);
		if (state.BuffersQueued) {
			return started ? State_Playing : State_Prepared;
		}
		return State_Stopped;
	}

	bool Update() {
		if (!started) {
			return true;
		}
		DWORD cbValid = std::min(BUFFER_SIZE, dataSize - currentPos);
		if (cbValid == 0) {
			return false;
		}
		XAUDIO2_VOICE_STATE state;
		sourceVoice->GetState(&state);
		if (state.BuffersQueued < MAX_BUFFER_COUNT - 1) {
			SetFilePointer(handle, dataOffset + currentPos, nullptr, FILE_BEGIN);
			if (!Read(handle, &buf[BUFFER_SIZE * curBuf], cbValid)) {
				return false;
			}
			XAUDIO2_BUFFER buffer = {};
			buffer.AudioBytes = cbValid;
			buffer.pAudioData = &buf[BUFFER_SIZE * curBuf];
			buffer.Flags = XAUDIO2_END_OF_STREAM;
			sourceVoice->SubmitSourceBuffer(&buffer, seekInfo.PacketCount ? &seekInfo : nullptr);
			currentPos += cbValid;
			curBuf = (curBuf + 1) % MAX_BUFFER_COUNT;
			if (state.BuffersQueued == 0) {
				sourceVoice->Start();
			}
		}
		return true;
	}

	static const size_t BUFFER_SIZE = 0x10000;
	static const int MAX_BUFFER_COUNT = 3;
	bool started;
	IXAudio2SourceVoice* sourceVoice;
	BufferType buf;
	std::vector<UINT32> seekTable;
	XAUDIO2_BUFFER_WMA seekInfo;
	ScopedHandle handle;
	size_t dataSize;
	size_t dataOffset;
	size_t currentPos;
	int curBuf;
};

/**
* Engineの実装.
*/
class EngineImpl : public Engine
{
public:
//	EngineImpl() : Engine(), xaudio(), masteringVoice(nullptr) {}
//	virtual ~EngineImpl() {}

	virtual bool Initialize() override {
		ComPtr<IXAudio2> tmpAudio;
		UINT32 flags = 0;
#ifndef NDEBUG
//		flags |= XAUDIO2_DEBUG_ENGINE;
#endif // NDEBUG
		HRESULT hr = XAudio2Create(&tmpAudio, flags);
		if (FAILED(hr)) {
			return false;
		}
		if (flags & XAUDIO2_DEBUG_ENGINE) {
			XAUDIO2_DEBUG_CONFIGURATION debug = {};
			debug.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
			debug.BreakMask = XAUDIO2_LOG_ERRORS;
			tmpAudio->SetDebugConfiguration(&debug);
		}
		hr = tmpAudio->CreateMasteringVoice(&masteringVoice);
		if (FAILED(hr)) {
			return false;
		}
		xaudio = tmpAudio;
		return true;
	}

	virtual void Destroy() override {
		streamSound.reset();
		soundList.clear();
		xaudio.Reset();
	}

	virtual bool Update() override {
		for (SoundList::iterator itr = soundList.begin(); itr != soundList.end();) {
			if (!itr->unique()) {
				++itr;
				continue;
			}
			XAUDIO2_VOICE_STATE state;
			SoundImpl* p = static_cast<SoundImpl*>(itr->get());
			p->sourceVoice->GetState(&state);
			if (state.BuffersQueued > 0) {
				++itr;
			} else {
				itr = soundList.erase(itr);
			}
		}
		if (streamSound) {
			if (!streamSound->Update()) {
				streamSound.reset();
			}
		}
		return true;
	}

	virtual SoundPtr Prepare(const wchar_t* filename) override {
		WAVEFORMATEX wfx;
		std::shared_ptr<SoundImpl> sound(new SoundImpl);
		if (!LoadWaveFile(filename, &wfx, &sound->file)) {
			return nullptr;
		}
		if (FAILED(xaudio->CreateSourceVoice(&sound->sourceVoice, &wfx))) {
			return nullptr;
		}

		sound->buffer = {};
		sound->buffer.pAudioData = sound->file.data();
		sound->buffer.Flags = XAUDIO2_END_OF_STREAM;
		sound->buffer.AudioBytes = sound->file.size();

		soundList.push_back(sound);
		return sound;
	}

	virtual SoundPtr PrepareStream(const wchar_t* filename) override {
		ScopedHandle hFile = CreateFile2(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
		if (!hFile) {
			return nullptr;
		}
		FILE_STANDARD_INFO fileInfo;
		if (!GetFileInformationByHandleEx(hFile, FileStandardInfo, &fileInfo, sizeof(fileInfo))) {
			return nullptr;
		}
		if (fileInfo.EndOfFile.HighPart > 0) {
			return nullptr;
		}
		if (fileInfo.EndOfFile.LowPart < (sizeof(RIFFChunk) * 2 + sizeof(DWORD) + sizeof(WAVEFORMAT))) {
			return nullptr;
		}

		WF wf;
		if (!GetWaveFormat(hFile, wf)) {
			return nullptr;
		}

		streamSound.reset(new StreamSoundImpl(hFile, wf.dataOffset, wf.dataSize));
		hFile.handle = 0;
		if (FAILED(xaudio->CreateSourceVoice(&streamSound->sourceVoice, &wf.waveFormat))) {
			return nullptr;
		}
		if (wf.seekSize) {
			streamSound->seekTable.resize(wf.seekSize);
			if (!Read(streamSound->handle, streamSound->seekTable.data(), wf.seekSize * 4)) {
				return nullptr;
			}
			for (auto& e : streamSound->seekTable) {
				e = _byteswap_ulong(e);
			}
			streamSound->seekInfo.PacketCount = wf.seekSize;
			streamSound->seekInfo.pDecodedPacketCumulativeBytes = streamSound->seekTable.data();
		}
		return streamSound;
	}

	virtual void SetMasterVolume(float vol) override {
		if (xaudio) {
			masteringVoice->SetVolume(vol);
		}
	}

private:
	ComPtr<IXAudio2> xaudio;
	IXAudio2MasteringVoice* masteringVoice;

	typedef std::list<SoundPtr> SoundList;
	SoundList soundList;
	std::shared_ptr<StreamSoundImpl> streamSound;
};

Engine& Engine::Get()
{
	static EngineImpl engine;
	return engine;
}

} // namespace Audio
