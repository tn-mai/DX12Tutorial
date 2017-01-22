/**
* @file Audio.cpp
*/
#include "Audio.h"
#include <xaudio2.h>
#include <vector>
#include <list>
#include <stdint.h>
#include <wrl/client.h>

#pragma comment(lib, "xaudio2.lib")

using Microsoft::WRL::ComPtr;

namespace Audio {

typedef std::vector<uint8_t> BufferType;

/**
* WAVƒf[ƒ^.
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
	HANDLE hFile = CreateFile2(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
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

/**
* Sound‚ÌŽÀ‘•.
*/
class SoundImpl : public Sound
{
public:
	SoundImpl() : started(false), sourceVoice(nullptr) {}
	virtual ~SoundImpl() {
		if (sourceVoice) {
			sourceVoice->DestroyVoice();
		}
	}
	virtual bool Play() {
		if (started) {
			sourceVoice->Stop();
			sourceVoice->FlushSourceBuffers();
			sourceVoice->SubmitSourceBuffer(&buffer);
		}
		started = true;
		return SUCCEEDED(sourceVoice->Start());
	}
	virtual bool Pause() {
		return SUCCEEDED(sourceVoice->Stop());
	}
	virtual bool Seek() {
		return true;
	}
	virtual bool Stop() {
		if (started) {
			started = false;
			return SUCCEEDED(sourceVoice->Stop());
		}
		return false;
	}
	virtual float SetVolume(float volume) {
		sourceVoice->SetVolume(volume);
		return volume;
	}
	virtual float SetPitch(float pitch) {
		sourceVoice->SetFrequencyRatio(pitch);
		return pitch;
	}
	virtual State GetState() const {
		XAUDIO2_VOICE_STATE state;
		sourceVoice->GetState(&state);
		if (state.BuffersQueued) {
			return started ? State_Playing : State_Prepared;
		}
		return State_Stopped;
	}

	bool started;
	IXAudio2SourceVoice* sourceVoice;
	BufferType file;
	XAUDIO2_BUFFER buffer;
};

/**
* Engine‚ÌŽÀ‘•.
*/
class EngineImpl : public Engine
{
public:
	EngineImpl() : Engine(), xaudio(), masteringVoice(nullptr) {}
	virtual ~EngineImpl() {}

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
		return true;
	}

	virtual SoundPtr Prepare(const wchar_t* filename) override {
		WavData wavData;
		std::shared_ptr<SoundImpl> sound(new SoundImpl);
		if (!LoadWavDataFromFile(filename, sound->file, wavData)) {
			return nullptr;
		}
		if (FAILED(xaudio->CreateSourceVoice(&sound->sourceVoice, wavData.wfx))) {
			return nullptr;
		}

		sound->buffer.pAudioData = wavData.startAudio;
		sound->buffer.Flags = XAUDIO2_END_OF_STREAM;
		sound->buffer.AudioBytes = wavData.audioBytes;
		if (wavData.loopLength > 0) {
			sound->buffer.LoopBegin = wavData.loopStart;
			sound->buffer.LoopLength = wavData.loopLength;
			sound->buffer.LoopCount = 1;
		}
		if (FAILED(sound->sourceVoice->SubmitSourceBuffer(&sound->buffer))) {
			return nullptr;
		}

		soundList.push_back(sound);
		return sound;
	}

	virtual void SetMasterVolume(float vol) override {
		if (masteringVoice) {
			masteringVoice->SetVolume(vol);
		}
	}

private:
	ComPtr<IXAudio2> xaudio;
	IXAudio2MasteringVoice* masteringVoice;

	typedef std::list<SoundPtr> SoundList;
	SoundList soundList;
};

Engine& Engine::Get()
{
	static EngineImpl engine;
	return engine;
}

} // namespace Audio
