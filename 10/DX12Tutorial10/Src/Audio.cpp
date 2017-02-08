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

using Microsoft::WRL::ComPtr;

namespace Audio {

const uint32_t FOURCC_RIFF_TAG = MAKEFOURCC('R', 'I', 'F', 'F');
const uint32_t FOURCC_FORMAT_TAG = MAKEFOURCC('f', 'm', 't', ' ');
const uint32_t FOURCC_DATA_TAG = MAKEFOURCC('d', 'a', 't', 'a');
const uint32_t FOURCC_WAVE_FILE_TAG = MAKEFOURCC('W', 'A', 'V', 'E');
const uint32_t FOURCC_XWMA_FILE_TAG = MAKEFOURCC('X', 'W', 'M', 'A');
const uint32_t FOURCC_XWMA_DPDS = MAKEFOURCC('d', 'p', 'd', 's');

struct RIFFChunk
{
	uint32_t tag;
	uint32_t size;
};

/**
* WAVデータ.
*/
struct WF
{
	union U {
		WAVEFORMATEXTENSIBLE ext;
		struct ADPCMWAVEFORMAT {
			WAVEFORMATEX    wfx;
			WORD            wSamplesPerBlock;
			WORD            wNumCoef;
			ADPCMCOEFSET    coef[7];
		} adpcm;
	} u;
	size_t dataOffset;
	size_t dataSize;
	size_t seekOffset;
	size_t seekSize;
};

typedef std::vector<uint8_t> BufferType;

struct ScopedHandle
{
	ScopedHandle(HANDLE h) : handle(h == INVALID_HANDLE_VALUE ? 0 : h) {}
	~ScopedHandle() { if (handle) { CloseHandle(handle); } }
	operator HANDLE() { return handle; }
	HANDLE handle;
};

bool Read(HANDLE hFile, void* buf, DWORD size)
{
	DWORD readSize;
	if (!ReadFile(hFile, buf, size, &readSize, nullptr) || readSize != size) {
		return false;
	}
	return true;
}

uint32_t GetWaveFormatTag(const WAVEFORMATEXTENSIBLE& wf)
{
	if (wf.Format.wFormatTag != WAVE_FORMAT_EXTENSIBLE) {
		return wf.Format.wFormatTag;
	}
	return wf.SubFormat.Data1;
}

// フォーマット情報を取得
bool LoadWaveFile(HANDLE hFile, WF& wf, std::vector<UINT32>& seekTable, std::vector<uint8_t>* source)
{
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
	if (fourcc != FOURCC_WAVE_FILE_TAG && fourcc != FOURCC_XWMA_FILE_TAG) {
		return false;
	}

	bool hasWaveFormat = false;
	bool hasData = false;
	bool hasDpds = false;
	size_t offset = 12;
	do {
		if (SetFilePointer(hFile, offset, nullptr, FILE_BEGIN) != offset) {
			return false;
		}

		RIFFChunk chunk;
		if (!Read(hFile, &chunk, sizeof(chunk))) {
			break;
		}

		if (chunk.tag == FOURCC_FORMAT_TAG) {
			if (!Read(hFile, &wf.u, std::min(chunk.size, sizeof(WF::U)))) {
				break;
			}
			switch (GetWaveFormatTag(wf.u.ext)) {
			case WAVE_FORMAT_PCM:
				wf.u.ext.Format.cbSize = 0;
				/* FALLTHROUGH */
			case WAVE_FORMAT_IEEE_FLOAT:
			case WAVE_FORMAT_ADPCM:
				wf.seekSize = 0;
				wf.seekOffset = 0;
				hasDpds = true;
				break;
			case WAVE_FORMAT_WMAUDIO2:
			case WAVE_FORMAT_WMAUDIO3:
				break;
			default:
				// このコードでサポートしないフォーマット.
				return false;
			}
			hasWaveFormat = true;
		}
		else if (chunk.tag == FOURCC_DATA_TAG) {
			wf.dataOffset = offset + sizeof(RIFFChunk);
			wf.dataSize = chunk.size;
			hasData = true;
		}
		else if (chunk.tag == FOURCC_XWMA_DPDS) {
			wf.seekOffset = offset + sizeof(RIFFChunk);
			wf.seekSize = chunk.size / 4;
			hasDpds = true;
		}
		offset += chunk.size + sizeof(RIFFChunk);
	} while (!hasWaveFormat || !hasData || !hasDpds);
	if (!(hasWaveFormat && hasData && hasDpds)) {
		return false;
	}

	if (wf.seekSize) {
		seekTable.resize(wf.seekSize);
		SetFilePointer(hFile, wf.seekOffset, nullptr, FILE_BEGIN);
		if (!Read(hFile, seekTable.data(), wf.seekSize * 4)) {
			return nullptr;
		}
		// XWMAはPowerPC搭載のXBOX360用に開発されたため、データはビッグエンディアンになっている.
		// X86はリトルエンディアンなので変換しなければならない.
		for (auto& e : seekTable) {
			e = _byteswap_ulong(e);
		}
	}
	if (source) {
		source->resize(wf.dataSize);
		SetFilePointer(hFile, wf.dataOffset, nullptr, FILE_BEGIN);
		if (!Read(hFile, source->data(), wf.dataSize)) {
			return false;
		}
	}
	return true;
}

/**
* Soundの実装.
*/
class SoundImpl : public Sound
{
public:
	SoundImpl() :
		state(State_Create), sourceVoice(nullptr) {}
	virtual ~SoundImpl() override {
		if (sourceVoice) {
			sourceVoice->DestroyVoice();
		}
	}
	virtual bool Play(int flags) override {
		if (!(state & State_Pausing)) {
			Stop();
			XAUDIO2_BUFFER buffer = {};
			buffer.Flags = XAUDIO2_END_OF_STREAM;
			buffer.AudioBytes = source.size();
			buffer.pAudioData = source.data();
			buffer.LoopCount = flags & Flag_Loop ? XAUDIO2_LOOP_INFINITE : XAUDIO2_NO_LOOP_REGION;
			if (seekTable.empty()) {
				if (FAILED(sourceVoice->SubmitSourceBuffer(&buffer))) {
					return false;
				}
			} else {
				const XAUDIO2_BUFFER_WMA seekInfo = { seekTable.data(), seekTable.size() };
				if (FAILED(sourceVoice->SubmitSourceBuffer(&buffer, &seekInfo))) {
					return false;
				}
			}
		}
		state = State_Playing;
		return SUCCEEDED(sourceVoice->Start());
	}
	virtual bool Pause() override {
		if (state & State_Playing) {
			state |= State_Pausing;
			return SUCCEEDED(sourceVoice->Stop());
		}
		return false;
	}
	virtual bool Seek() override {
		return true;
	}
	virtual bool Stop() override {
		if (state & State_Playing) {
			if (!(state & State_Pausing) && FAILED(sourceVoice->Stop())) {
				return false;
			}
			state = State_Stopped;
			return SUCCEEDED(sourceVoice->FlushSourceBuffers());
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
	virtual int GetState() const override {
		XAUDIO2_VOICE_STATE s;
		sourceVoice->GetState(&s);
		return s.BuffersQueued ? state : (State_Stopped | State_Prepared);
	}

	int state;
	IXAudio2SourceVoice* sourceVoice;
	std::vector<uint8_t> source;
	std::vector<UINT32> seekTable;
};

/**
* Soundの実装.
*/
class StreamSoundImpl : public Sound
{
public:
	StreamSoundImpl() = delete;
	explicit StreamSoundImpl(HANDLE h) :
		sourceVoice(nullptr), handle(h), state(State_Create), loop(false), currentPos(0), curBuf(0)
	{
		buf.resize(BUFFER_SIZE * MAX_BUFFER_COUNT);
	}
	virtual ~StreamSoundImpl() override {
		if (sourceVoice) {
			sourceVoice->DestroyVoice();
		}
	}
	virtual bool Play(int flags) override {
		if (!(state & State_Pausing)) {
			Stop();
		}
		state = State_Playing;
		loop = flags & Flag_Loop;
		return SUCCEEDED(sourceVoice->Start());
	}
	virtual bool Pause() override {
		if (state & State_Playing && !(state & State_Pausing)) {
			state |= State_Pausing;
			return SUCCEEDED(sourceVoice->Stop());
		}
		return false;
	}
	virtual bool Seek() override {
		return true;
	}
	virtual bool Stop() override {
		if (state & State_Playing) {
			if (!(state & State_Pausing) && FAILED(sourceVoice->Stop())) {
				return false;
			}
			state = State_Stopped;
			return SUCCEEDED(sourceVoice->FlushSourceBuffers());
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
	virtual int GetState() const override {
		XAUDIO2_VOICE_STATE s;
		sourceVoice->GetState(&s);
		return s.BuffersQueued ? (state | State_Prepared) : State_Stopped;
	}

	bool Update() {
		const DWORD cbValid = std::min(BUFFER_SIZE, dataSize - currentPos);
		if (cbValid == 0) {
			return false;
		}
		XAUDIO2_VOICE_STATE state;
		sourceVoice->GetState(&state);
		if (state.BuffersQueued < MAX_BUFFER_COUNT - 1) {
			SetFilePointer(handle, dataOffset + currentPos, nullptr, FILE_BEGIN);

			XAUDIO2_BUFFER buffer = {};
			buffer.pAudioData = &buf[BUFFER_SIZE * curBuf];
			buffer.Flags = cbValid == BUFFER_SIZE ? 0 : XAUDIO2_END_OF_STREAM;
			if (seekTable.empty()) {
				buffer.AudioBytes = cbValid;
				if (!Read(handle, &buf[BUFFER_SIZE * curBuf], cbValid)) {
					return false;
				}
				sourceVoice->SubmitSourceBuffer(&buffer, nullptr);
				currentPos += cbValid;
			}
			else {
				XAUDIO2_BUFFER_WMA bufWma = {};
				bufWma.PacketCount = cbValid / packetSize;
				bufWma.pDecodedPacketCumulativeBytes = seekTable.data() + (currentPos / packetSize);
				buffer.AudioBytes = bufWma.PacketCount * packetSize;
				if (!Read(handle, &buf[BUFFER_SIZE * curBuf], buffer.AudioBytes)) {
					return false;
				}
				sourceVoice->SubmitSourceBuffer(&buffer, &bufWma);
				currentPos += buffer.AudioBytes;
			}
			curBuf = (curBuf + 1) % MAX_BUFFER_COUNT;
			if (loop && currentPos >= dataSize) {
				currentPos = 0;
			}
		}
		return true;
	}

	IXAudio2SourceVoice* sourceVoice;
	std::vector<UINT32> seekTable;
	ScopedHandle handle;
	size_t dataSize;
	size_t dataOffset;
	size_t packetSize;

	static const size_t BUFFER_SIZE = 0x10000;
	static const int MAX_BUFFER_COUNT = 3;

	int state;
	bool loop;
	std::vector<uint8_t> buf;
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
		if (1) {
			XAUDIO2_DEBUG_CONFIGURATION debug = {};
			debug.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS | XAUDIO2_LOG_MEMORY;
			debug.BreakMask = XAUDIO2_LOG_ERRORS;
			debug.LogFunctionName = TRUE;
			tmpAudio->SetDebugConfiguration(&debug);
		}
		hr = tmpAudio->CreateMasteringVoice(&masteringVoice);
		if (FAILED(hr)) {
			return false;
		}
		xaudio.Swap(std::move(tmpAudio));
		return true;
	}

	virtual void Destroy() override {
		streamSound.reset();
		soundList.clear();
		xaudio.Reset();
	}

	virtual bool Update() override {
		soundList.remove_if(
			[](const SoundList::value_type& p) { return (p.use_count() <= 1) && (p->GetState() & State_Stopped); }
		);
		if (streamSound) {
			streamSound->Update();
			if ((streamSound.use_count() <= 1) && (streamSound->GetState() & State_Stopped)) {
				streamSound.reset();
			}
		}
		return true;
	}

	virtual SoundPtr Prepare(const wchar_t* filename) override {
		ScopedHandle hFile = CreateFile2(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
		if (!hFile) {
			return nullptr;
		}
		WF wf;
		std::shared_ptr<SoundImpl> sound(new SoundImpl);
		if (!LoadWaveFile(hFile, wf, sound->seekTable, &sound->source)) {
			return nullptr;
		}
		if (FAILED(xaudio->CreateSourceVoice(&sound->sourceVoice, &wf.u.ext.Format))) {
			return nullptr;
		}
		soundList.push_back(sound);
		return sound;
	}

	virtual SoundPtr PrepareStream(const wchar_t* filename) override {
		streamSound.reset(new StreamSoundImpl(CreateFile2(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr)));
		if (!streamSound->handle) {
			return nullptr;
		}
		WF wf;
		if (!LoadWaveFile(streamSound->handle, wf, streamSound->seekTable, nullptr)) {
			return nullptr;
		}
		if (FAILED(xaudio->CreateSourceVoice(&streamSound->sourceVoice, &wf.u.ext.Format))) {
			return nullptr;
		}
		streamSound->dataOffset = wf.dataOffset;
		streamSound->dataSize = wf.dataSize;
		streamSound->packetSize = wf.u.ext.Format.nBlockAlign;
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

	typedef std::list<std::shared_ptr<SoundImpl>> SoundList;
	SoundList soundList;
	std::shared_ptr<StreamSoundImpl> streamSound;
};

Engine& Engine::Get()
{
	static EngineImpl engine;
	return engine;
}

} // namespace Audio
