/**
* @file GamePad.cpp
*/
#include "GamePad.h"
#include <Windows.h>
#include <Xinput.h>
#include <vector>

enum VibrationType
{
	VibrationType_High,
	VibrationType_Low,
	countof_VibrationType
};

struct VibrationEvent
{
	float deltaTime;
	VibrationType type;
	float gateTime;
	float velocity;
};
typedef std::vector<VibrationEvent> VibrationSequence;

struct VibrationUnit
{
	float currentTime;
	float gateTime;
	float startVelocity;
	float targetVelocity;
};

struct VibrationState
{
	int sequenceNo;
	size_t index;
	float deltaTime;
	VibrationUnit unit[countof_VibrationType];
};

static const VibrationEvent vib00[] = {
	{ 0.0f, VibrationType_High, 0.25f, 1.0f },
	{ 0.25f, VibrationType_High, 0.25f, 0.75f },
	{ 0.0f, VibrationType_Low, 0.25f, 1.0f },
	{ 0.25f, VibrationType_Low, 0.75f, 0.0f },
	{ 0.5f, VibrationType_High, 0.5f, 0.0f },
};

static const VibrationEvent vib01[] = {
	{ 0.0f, VibrationType_Low, 0.1f, 0.5f },
	{ 0.1f, VibrationType_Low, 0.1f, 0.2f },
	{ 0.1f, VibrationType_Low, 0.1f, 0.0f },
};

static VibrationState vibrationState[countof_GamePadId];
static std::vector<VibrationSequence> vibrationList;

/**
* ゲームパッドの初期化.
*/
void InitGamePad()
{
	for (auto& e : vibrationState) {
		e = {};
		e.sequenceNo = -1;
	}
	vibrationList.resize(2);
	vibrationList[0].assign(vib00, vib00 + _countof(vib00));
	vibrationList[1].assign(vib01, vib01 + _countof(vib01));
}

/**
*
*/
void VibrateGamePad(uint32_t id, uint32_t seqNo)
{
	if (id < countof_GamePadId && seqNo < vibrationList.size()) {
		vibrationState[id].sequenceNo = seqNo;
		vibrationState[id].index = 0;
		vibrationState[id].deltaTime = 0.0f;
	}
}

/**
*
*/
WORD GetVibration(const VibrationUnit& unit)
{
	float ratio = 1.0f;
	if (unit.currentTime < unit.gateTime) {
		ratio = unit.currentTime / unit.gateTime;
	}
	return static_cast<WORD>(unit.startVelocity + (unit.targetVelocity - unit.startVelocity) * ratio * 65535.0f);
}

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
void UpdateGamePad(double delta)
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
			}
			else if (state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
				gamepad.buttons |= GamePad::DPAD_RIGHT;
			}
			if (state.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
				gamepad.buttons |= GamePad::DPAD_DOWN;
			}
			else if (state.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
				gamepad.buttons |= GamePad::DPAD_UP;
			}
		}
		gamepad.trigger = ~gamepad.prevButtons & (gamepad.prevButtons ^ gamepad.buttons);
		gamepad.prevButtons = gamepad.buttons;

		VibrationState& vibState = vibrationState[id];
		if (vibState.sequenceNo < 0) {
			XINPUT_VIBRATION vib = {};
			XInputSetState(id, &vib);
		} else {
			const VibrationSequence& vibSeq = vibrationList[vibState.sequenceNo];
			vibState.deltaTime += static_cast<float>(delta);
			for (; vibState.index < vibSeq.size(); ++vibState.index) {
				const VibrationEvent& vibEvent = vibSeq[vibState.index];
				if (vibState.deltaTime < vibEvent.deltaTime) {
					vibState.unit[vibEvent.type].currentTime += static_cast<float>(delta);
					break;
				}
				vibState.deltaTime -= vibEvent.deltaTime;
				vibState.unit[vibEvent.type].currentTime = 0.0f;
				vibState.unit[vibEvent.type].gateTime = vibEvent.gateTime;
				vibState.unit[vibEvent.type].startVelocity = vibState.unit[vibEvent.type].targetVelocity;
				vibState.unit[vibEvent.type].targetVelocity = vibEvent.velocity;
			}

			XINPUT_VIBRATION vib = {
				GetVibration(vibState.unit[VibrationType_High]),
				GetVibration(vibState.unit[VibrationType_Low])
			};
			XInputSetState(id, &vib);
		}
	}
}