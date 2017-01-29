/**
* @file GamePad.cpp
*/
#include "GamePad.h"

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
		static GamePad dummy = {};
		dummy.buttons = 0;
		return dummy;
	}
	return gamepad[id];
}
