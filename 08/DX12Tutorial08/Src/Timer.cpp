/**
* @file Timer.cpp
*/
#include "Timer.h"
#include <Windows.h>

/**
* �R���X�g���N�^.
*/
Timer::Timer() :
	fpsDelta(0),
	fpsFrames(0),
	fps(30)
{
	LARGE_INTEGER i;
	QueryPerformanceFrequency(&i);
	frequency = static_cast<double>(i.QuadPart);
	QueryPerformanceCounter(&i);
	lastFrameTime = i.QuadPart;
}

/**
* �O��̌v������̌o�ߎ��Ԃ��擾����.
*
* @return �o�ߎ���.
*/
double Timer::GetFrameDelta()
{
	LARGE_INTEGER i;
	QueryPerformanceCounter(&i);
	const double delta = static_cast<double>(i.QuadPart - lastFrameTime) / frequency;

	++fpsFrames;
	fpsDelta += delta;
	if (fpsDelta > 1.0) {
		fps = fpsFrames * 0.5 + fps * 0.5;
		fpsFrames = 0;
		fpsDelta -= 1.0;
	}
	lastFrameTime = i.QuadPart;
	return delta;
}

/**
* FPS���擾����.
*
* @return FPS�l.
*/
double Timer::GetFPS() const
{
	return fps;
}
