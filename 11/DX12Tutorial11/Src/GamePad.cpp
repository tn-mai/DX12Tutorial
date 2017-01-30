/**
* @file GamePad.cpp
*/
#include "GamePad.h"
#include <Windows.h>
#include <Xinput.h>
#include <random>
#include <vector>

namespace /* unnamed */ {

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

static const VibrationEvent vib02[] = {
	{ 0.0f, VibrationType_Low, 0.5f, 1.0f },
	{ 0.5f, VibrationType_Low, 1.0f, 0.0f },
};

static const VibrationEvent vib03[] = {
	{ 0.0f, VibrationType_High, 0.5f, 1.0f },
	{ 0.5f, VibrationType_High, 1.0f, 0.0f },
};

static VibrationState vibrationState[countof_GamePadId];
static std::vector<VibrationSequence> vibrationList;

struct ConversionData
{
	uint32_t di;
	uint32_t gp;
};

const ConversionData convMap[] = {
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

const float minInterval = 3.0f; ///< 最小待ち時間. 
const float maxInterval = 5.0f; ///< 最大待ち時間.

std::mt19937 random;

/**
* XBOX360コントローラーの状態をGamePad構造体に反映する.
*
* @param id    コントローラーID.
* @param delta 前回の呼び出しからの経過時間.
*/
void ApplyControllerState(uint32_t id, float delta)
{
	GamePad& gamepad = GetGamePad(id);
	if (gamepad.connectionCheckInterval > 0.0f) {
		gamepad.connectionCheckInterval -= delta;
		return;
	}
	gamepad.connectionCheckInterval = 0;

	XINPUT_STATE state;
	if (XInputGetState(id, &state) != ERROR_SUCCESS) {
		if (gamepad.buttons & GamePad::CONNECTED) {
			OutputDebugStringA("DISCONNECTED\n");
		}
		const std::uniform_real_distribution<float> intervalRange(minInterval, maxInterval);
		gamepad.connectionCheckInterval = intervalRange(random);
		gamepad.buttons &= ~GamePad::CONNECTED;
		return;
	}

	if (!(gamepad.buttons & GamePad::CONNECTED)) {
		OutputDebugStringA("CONNECTED\n");
	}
	gamepad.buttons = GamePad::CONNECTED;
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

} // unnamed namespace
  
/**
* ゲームパッドの初期化.
*/
void InitGamePad()
{
	random.seed(std::random_device()());
	for (auto& e : vibrationState) {
		e = {};
		e.sequenceNo = -1;
	}
	vibrationList.resize(4);
	vibrationList[0].assign(vib00, vib00 + _countof(vib00));
	vibrationList[1].assign(vib01, vib01 + _countof(vib01));
	vibrationList[2].assign(vib02, vib02 + _countof(vib02));
	vibrationList[3].assign(vib03, vib03 + _countof(vib03));
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
* 振動状態を更新.
*
* @param unit 更新する振動ユニット.
* @param delta 前回の呼び出しからの経過時間.
*
* @return 振動パラメータ.
*/
WORD UpdateVibration(VibrationUnit& unit, float delta)
{
	float ratio = 1.0f;
	if (unit.currentTime + delta < unit.gateTime) {
		unit.currentTime += delta;
		ratio = unit.currentTime / unit.gateTime;
	}
	return static_cast<WORD>((unit.startVelocity + (unit.targetVelocity - unit.startVelocity) * ratio) * 65535.0f);
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
*
* @param delta 前回の呼び出しからの経過時間.
*/
void UpdateGamePad(float delta)
{
	for (uint32_t id = 0; id < countof_GamePadId; ++id) {
		ApplyControllerState(id, delta);
		GamePad& gamepad = GetGamePad(id);
		gamepad.buttonDown = ~gamepad.prevButtons & gamepad.buttons;
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
				UpdateVibration(vibState.unit[VibrationType_Low], delta),
				UpdateVibration(vibState.unit[VibrationType_High], delta)
			};
			XInputSetState(id, &vib);
		}
	}
}