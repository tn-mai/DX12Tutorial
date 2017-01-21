/**
* @file GamePad.cpp
*/
#include "GamePad.h"

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
