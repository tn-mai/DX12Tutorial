/**
* @file Audio.h
*/
#ifndef DX12TUTORIAL_SRC_AUDIO_H_
#define DX12TUTORIAL_SRC_AUDIO_H_
#include <memory>

namespace Audio {

enum State {
	State_Create = 0x01,
	State_Preparing = 0x02,
	State_Prepared = 0x03,
	State_Playing = 0x04,
	State_Stopped = 0x05,
	State_Paused = 0x10,
	State_Failed = 0x20,
};

class Sound
{
public:
	virtual ~Sound() {}
	virtual bool Play() = 0;
	virtual bool Pause() = 0;
	virtual bool Seek() = 0;
	virtual bool Stop() = 0;
	virtual float SetVolume(float) = 0;
	virtual float SetPitch(float) = 0;
	virtual State GetState() const = 0;
};
typedef std::shared_ptr<Sound> SoundPtr;

class Engine
{
public:
	static Engine& Get();
	
	Engine() = default;

	virtual ~Engine() {}
	virtual bool Initialize() = 0;
	virtual void Destroy() = 0;
	virtual bool Update() = 0;
	virtual SoundPtr Prepare(const wchar_t*) = 0;
	virtual void SetMasterVolume(float) = 0;

private:
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;
};

} // namespace Audio

#endif // DX12TUTORIAL_SRC_AUDIO_H_