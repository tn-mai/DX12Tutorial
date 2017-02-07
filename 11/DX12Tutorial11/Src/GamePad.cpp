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
* ���[�^�[�̎��.
*/
enum VibrationType
{
	VibrationType_Low, ///< ����g���[�^�[.
	VibrationType_High, ///< �����g���[�^�[.
	countof_VibrationType ///< ���[�^�[�̎�ސ�.
};

/**
* �U���C�x���g.
*/
struct VibrationEvent
{
	float deltaTime; ///< �J�n����.
	VibrationType type; ///< ���[�^�[�̎��.
	float gateTime; ///< �U������.
	float velocity; ///< �U���̋���.
};
typedef std::vector<VibrationEvent> VibrationSequence; ///< �U���V�[�P���X.

/**
* �U������p�����[�^.
*/
struct VibrationUnit
{
	float currentTime; ///< �U���o�ߎ���.
	float gateTime; ///< ���U������.
	float startVelocity; ///< �U���J�n���̋���.
	float targetVelocity; ///< �U���I�����̋���.
};

/**
* �U�����.
*/
struct VibrationState
{
	VibrationState() : sequenceNo(-1) {}

	int sequenceNo; ///< �U���V�[�P���X�̔ԍ�.
	size_t index; ///< �V�[�P���X���̍Đ��ʒu.
	float deltaTime; ///< �Đ�����.
	VibrationUnit unit[countof_VibrationType]; ///< ���[�^�[���̃p�����[�^.
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

const float minInterval = 3.0f; ///< �ŏ��҂�����. 
const float maxInterval = 5.0f; ///< �ő�҂�����.

std::mt19937 random;

/**
* XBOX360�R���g���[���[�̏�Ԃ�GamePad�\���̂ɔ��f����.
*
* @param id    �R���g���[���[ID.
* @param delta �O��̌Ăяo������̌o�ߎ���.
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
* �U����Ԃ��X�V.
*
* @param unit �X�V����U�����j�b�g.
* @param delta �O��̌Ăяo������̌o�ߎ���.
*
* @return �U���p�����[�^.
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
* �U����Ԃ��X�V.
*
* @param unit �X�V����U�����j�b�g.
* @param delta �O��̌Ăяo������̌o�ߎ���.
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
* �Q�[���p�b�h�̏�����.
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
* �Q�[���p�b�h�̏�Ԃ��擾����.
*
* @param id �擾����Q�[���p�b�h�̃C���f�b�N�X.
*           ���݂�0-1���L��. 0-1�ȊO�̒l��n���ƃ_�~�[�f�[�^��Ԃ�.
*           �_�~�[�f�[�^�͏�ɉ��̓��͂��Ȃ���ԂɂȂ��Ă���.
*
* @return id�ɑΉ�����Q�[���p�b�h���.
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
* �Q�[���p�b�h�̏�Ԃ��X�V����.
*
* @param delta �O��̌Ăяo������̌o�ߎ���.
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
* �R���g���[���[��U��������.
*
* @param id    �R���g���[���[ID.
* @param seqNo �U���V�[�P���XNo.
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
* �U���V�[�P���X���X�g�̗v�f�����擾����.
*
* @return �U���V�[�P���X���X�g�̗v�f��.
*/
size_t GetVibrationListSize()
{
	return vibrationList.size();
}
