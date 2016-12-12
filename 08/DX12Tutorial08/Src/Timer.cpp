/**
* @file Timer.cpp
*/
#include "Timer.h"
#include <Windows.h>

/**
* コンストラクタ.
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
* 前回の計測からの経過時間を取得する.
*
* @return 経過時間.
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
* FPSを取得する.
*
* @return FPS値.
*/
double Timer::GetFPS() const
{
	return fps;
}
