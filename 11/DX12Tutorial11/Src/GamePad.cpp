/**
* @file GamePad.cpp
*/
#include "GamePad.h"
#include <Windows.h>
#include <Xinput.h>

/**
* ゲームパッドの状態を取得する.
*
* @param id 取得するゲームパッドのインデックス.
*           現在は0-1が有効. 0-1以外の値を渡すとダミーデータを返す.
*           ダミーデータは常に何の入力もない状態になっている.
*
* @return idに対応するゲームパッド状態.
*/
GamePad& GetGamePad(uint32_t id) {
	static GamePad gamepad[countof_GamePadId] = {};
	if (id >= countof_GamePadId) {
		static GamePad dummy = {};
		dummy.buttons = 0;
		return dummy;
	}
	return gamepad[id];
}

/**
* ゲームパッドの状態を更新する.
*/
void UpdateGamePad()
{
	static const struct { uint32_t di; uint32_t gp; } convMap[] = {
		{ XINPUT_GAMEPAD_DPAD_UP, GamePad::DPAD_UP },
		{ XINPUT_GAMEPAD_DPAD_DOWN, GamePad::DPAD_DOWN },
		{ XINPUT_GAMEPAD_DPAD_LEFT, GamePad::DPAD_LEFT },
		{ XINPUT_GAMEPAD_DPAD_RIGHT, GamePad::DPAD_RIGHT },
		{ XINPUT_GAMEPAD_START, GamePad::START },
		{ XINPUT_GAMEPAD_A, GamePad::A },
		{ XINPUT_GAMEPAD_B, GamePad::B },
		{ XINPUT_GAMEPAD_X, GamePad::X },
		{ XINPUT_GAMEPAD_Y, GamePad::Y },
		{ XINPUT_GAMEPAD_LEFT_SHOULDER, GamePad::L },
		{ XINPUT_GAMEPAD_RIGHT_SHOULDER, GamePad::R },
	};

	for (uint32_t id = 0; id < countof_GamePadId; ++id) {
		GamePad& gamepad = GetGamePad(id);
		XINPUT_STATE state;
		if (XInputGetState(id, &state) == ERROR_SUCCESS) {
			gamepad.buttons = 0;
			for (auto e : convMap) {
				if (state.Gamepad.wButtons & e.di) {
					gamepad.buttons |= e.gp;
				}
			}
			if (state.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
				gamepad.buttons |= GamePad::DPAD_LEFT;
			} else if (state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
				gamepad.buttons |= GamePad::DPAD_RIGHT;
			}
			if (state.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
				gamepad.buttons |= GamePad::DPAD_DOWN;
			} else if (state.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
				gamepad.buttons |= GamePad::DPAD_UP;
			}
		}
		gamepad.trigger = ~gamepad.prevButtons & (gamepad.prevButtons ^ gamepad.buttons);
		gamepad.prevButtons = gamepad.buttons;
	}
}