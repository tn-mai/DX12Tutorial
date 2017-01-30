/**
* @file GamePad.h
*/
#ifndef DX12TUTORIAL_SRC_GAMEPAD_H_
#define DX12TUTORIAL_SRC_GAMEPAD_H_
#include <stdint.h>

/**
* ゲームパッド入力を模した構造体.
*/
struct GamePad
{
	enum {
		DPAD_UP = 0x0001,
		DPAD_DOWN = 0x0002,
		DPAD_LEFT = 0x0004,
		DPAD_RIGHT = 0x0008,
		START = 0x0010,
		A = 0x0020,
		B = 0x0040,
		X = 0x0080,
		Y = 0x0100,
		L = 0x0200,
		R = 0x0400,
		CONNECTED = 0x80000000,
	};
	uint32_t buttons;
	uint32_t buttonDown;
	uint32_t prevButtons;
	float connectionCheckInterval;
};

enum GamePadId
{
	GamePadId_1P,
	GamePadId_2P,
	countof_GamePadId
};

GamePad& GetGamePad(uint32_t id);
void InitGamePad();
void UpdateGamePad(float delta);
void VibrateGamePad(uint32_t id, uint32_t seqNo);
size_t GetVibrationListSize();

#endif // DX12TUTORIAL_SRC_GAMEPAD_H_