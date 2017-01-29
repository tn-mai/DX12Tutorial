/**
* @file Timer.h
*/
#ifndef DX12TUTORIAL_SRC_TIMER_H_
#define DX12TUTORIAL_SRC_TIMER_H_
#include <stdint.h>

/**
* �o�ߎ��Ԍv���N���X.
*/
class Timer
{
public:
	Timer();
	double GetFrameDelta();
	double GetFPS() const;

private:
	double frequency;
	int64_t lastFrameTime;
	double fpsDelta;
	double fpsFrames;
	double fps;
};

#endif // DX12TUTORIAL_SRC_TIMER_H_