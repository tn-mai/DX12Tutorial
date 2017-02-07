/**
* @file GamePad.cpp
*/
#include "GamePad.h"
#include <Windows.h>
#include <Xinput.h>
#include <random>
#include <vector>
#include <iterator>

namespace /* unnamed */ {

/**
* モーターの種類.
*/
enum VibrationType
{
	VibrationType_Low, ///< 低周波モーター.
	VibrationType_High, ///< 高周波モーター.
	countof_VibrationType ///< モーターの種類数.
};

/**
* 振動イベント.
*/
struct VibrationEvent
{
	float deltaTime; ///< 開始時間.
	VibrationType type; ///< モーターの種類.
	float gateTime; ///< 振動時間.
	float velocity; ///< 振動の強さ.
};
typedef std::vector<VibrationEvent> VibrationSequence; ///< 振動シーケンス.

/**
* 振動制御パラメータ.
*/
struct VibrationUnit
{
	float currentTime; ///< 振動経過時間.
	float gateTime; ///< 総振動時間.
	float startVelocity; ///< 振動開始時の強さ.
	float targetVelocity; ///< 振動終了時の強さ.
};

/**
* 振動状態.
*/
struct VibrationState
{
	VibrationState() : sequenceNo(-1) {}

	int sequenceNo; ///< 振動シーケンスの番号.
	size_t index; ///< シーケンス内の再生位置.
	float deltaTime; ///< 再生時間.
	VibrationUnit unit[countof_VibrationType]; ///< モーター毎のパラメータ.
};

static const VibrationEvent vib00[] = {
	{ 0.0f, VibrationType_Low, 0.5f, 1.0f },
	{ 1.0f, VibrationType_Low, 1.0f, 0.0f },
};

static const VibrationEvent vib01[] = {
	{ 0.0f, VibrationType_High, 0.25f, 1.0f },
	{ 0.25f, VibrationType_High, 0.75f, 0.0f },
};

static const VibrationEvent vib02[] = {
	{ 0.0f, VibrationType_Low, 0.5f, 1.0f },
	{ 0.5f, VibrationType_Low, 0.5f, 0.1f },
	{ 0.5f, VibrationType_Low, 0.5f, 1.0f },
	{ 0.5f, VibrationType_Low, 1.0f, 0.0f },
};

static const VibrationEvent vib03[] = {
	{ 0.0f, VibrationType_High, 0.25f, 1.0f },
	{ 0.25f, VibrationType_High, 0.25f, 0.75f },
	{ 0.0f, VibrationType_Low, 0.25f, 1.0f },
	{ 0.25f, VibrationType_Low, 0.75f, 0.0f },
	{ 0.5f, VibrationType_High, 0.5f, 0.0f },
};

static const VibrationEvent vib04[] = {
	{ 0.0f, VibrationType_High, 0.1f, 0.5f },
	{ 0.1f, VibrationType_High, 0.1f, 0.2f },
	{ 0.1f, VibrationType_High, 0.1f, 0.0f },
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

/**
* 振動状態を更新.
*
* @param unit 更新する振動ユニット.
* @param delta 前回の呼び出しからの経過時間.
*
* @return 振動パラメータ.
*/
WORD UpdateVibrationUnit(VibrationUnit& unit, float delta)
{
	float ratio = 1.0f;
	if (unit.currentTime + delta < unit.gateTime) {
		unit.currentTime += delta;
		ratio = unit.currentTime / unit.gateTime;
	}
	return static_cast<WORD>((unit.startVelocity + (unit.targetVelocity - unit.startVelocity) * ratio) * 65535.0f);
}

/**
* 振動状態を更新.
*
* @param unit 更新する振動ユニット.
* @param delta 前回の呼び出しからの経過時間.
*/
void UpdateVibration(uint32_t id, float delta)
{
	VibrationState& vibState = vibrationState[id];
	if (vibState.sequenceNo < 0) {
		XINPUT_VIBRATION vib = {};
		XInputSetState(id, &vib);
		return;
	}
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
		UpdateVibrationUnit(vibState.unit[VibrationType_Low], delta),
		UpdateVibrationUnit(vibState.unit[VibrationType_High], delta)
	};
	XInputSetState(id, &vib);
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
	static const struct {
		const VibrationEvent* begin;
		const VibrationEvent* end;
	} seqList[] = {
		{ std::begin(vib00), std::end(vib00) },
		{ std::begin(vib01), std::end(vib01) },
		{ std::begin(vib02), std::end(vib02) },
		{ std::begin(vib03), std::end(vib03) },
		{ std::begin(vib04), std::end(vib04) },
	};
	vibrationList.reserve(_countof(seqList));
	for (const auto& e : seqList) {
		vibrationList.emplace_back(e.begin, e.end);
	}
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
		static GamePad dummy;
		dummy = {};
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
		UpdateVibration(id, delta);
	}
}

/**
* コントローラーを振動させる.
*
* @param id    コントローラーID.
* @param seqNo 振動シーケンスNo.
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
* 振動シーケンスリストの要素数を取得する.
*
* @return 振動シーケンスリストの要素数.
*/
size_t GetVibrationListSize()
{
	return vibrationList.size();
}
