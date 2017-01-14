/**
* @file MainGameScene.cpp
*/
#include "MainGameScene.h"
#include "../PSO.h"
#include "../GamePad.h"
#include <DirectXMath.h>
#include <algorithm>

using namespace DirectX;

struct MainGameScene::FormationData {
	uint32_t actionId;
	float interval;
	XMFLOAT2 offset;
};

struct MainGameScene::Occurrence {
	uint32_t time;
	XMFLOAT3 pos;
	const FormationData* begin;
	const FormationData* end;
};

namespace /* unnamed */ {

/**
* スプライト用セルデータ.
*/
const Sprite::Cell cellList[] = {
	{ XMFLOAT2(0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT2(800, 600) },

	{ XMFLOAT2(16.0f / 1024.0f, 48.0f / 512.0f), XMFLOAT2(480.0f / 1024.0f, 256.0f / 512.0f), XMFLOAT2(480, 256) },
};

const MainGameScene::FormationData formationA[] = {
	{ 2, 0.25f * 0, { 0, 0 } },
	{ 2, 0.25f * 1, { 0, 0 } },
	{ 2, 0.25f * 2, { 0, 0 } },
	{ 2, 0.25f * 3, { 0, 0 } },
	{ 2, 0.25f * 4, { 0, 0 } }
};

const MainGameScene::FormationData formationB[] = {
	{ 3, 0.25f * 0,{ 0, 0 } },
	{ 3, 0.25f * 1,{ 0, 0 } },
	{ 3, 0.25f * 2,{ 0, 0 } },
	{ 3, 0.25f * 3,{ 0, 0 } },
	{ 3, 0.25f * 4,{ 0, 0 } }
};

const MainGameScene::FormationData formationC[] = {
	{ 0, 0.25f * 0,{ 0, 0 } },
	{ 0, 0.25f * 1,{ 0, 0 } },
	{ 0, 0.25f * 2,{ 0, 0 } },
	{ 0, 0.25f * 3,{ 0, 0 } },
	{ 0, 0.25f * 4,{ 0, 0 } }
};

#define OCC_I(t, x, y, z, form) { t, {x, y, z}, form, form + _countof(form) }
#define OCC(t, x, y, z, form) OCC_I(t, x, y, z, formation##form)

const MainGameScene::Occurrence occurrenceList[] = {
	OCC( 5, 600, 0, 0.5f, A),
	OCC(10, 200, 0, 0.5f, B),
	OCC(15, 300, 0, 0.5f, C),
	OCC(20, 600, 0, 0.5f, C),
	OCC(30, 600, 0, 0.5f, A),
	OCC(30, 200, 0, 0.5f, B),
};

// 用途別スプライト数.
const size_t playerCount = 1;
const size_t playerShotCount = 3 * 2 * 3;
const size_t playerSpriteCount = playerCount + playerShotCount;

const size_t enemyCount = 128;
const size_t enemyShotCount = enemyCount * 16;
const size_t enemySpriteCount = enemyCount + enemyShotCount;

// 用途別スプライト配列中の開始インデックス.
const size_t PID_Player = 0;
const size_t PID_PlayerShot = PID_Player + playerCount;
const size_t EID_Enemy = 0;
const size_t EID_EnemyShot = EID_Enemy + enemyCount;

} // unnamed namespace

/**
*
*/
::Scene::ScenePtr MainGameScene::Create()
{
	return ::Scene::ScenePtr(new MainGameScene);
}

/**
* コンストラクタ.
*/
MainGameScene::MainGameScene() : Scene(L"MainGame")
{
}

/**
*
*/
bool MainGameScene::Load()
{
	::Scene::Graphics& graphics = ::Scene::Graphics::Get();
	Resource::ResourceLoader loader;
	if (!loader.Begin(graphics.csuDescriptorHeap)) {
		return false;
	}
	if (!loader.LoadFromFile(texBackground, 0, L"Res/UnknownPlanet.png")) {
		return false;
	}
	if (!loader.LoadFromFile(texObjects, 1, L"Res/Objects.png")) {
		return false;
	}
	if (!loader.LoadFromFile(texFont, 2, L"Res/TextFont.png")) {
		return false;
	}
	ID3D12CommandList* ppCommandLists[] = { loader.End() };
	graphics.commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	cellFile[0] = Sprite::LoadFromJsonFile(L"Res/Cell/CellFont.json");
	cellFile[1] = Sprite::LoadFromJsonFile(L"Res/Cell/CellEnemy.json");
	cellPlayer = Sprite::LoadFromJsonFile(L"Res/Cell/CellPlayer.json");

	anmOthers = LoadAnimationFromJsonFile(L"Res/Anm/AnmTitle.json");
	anmObjects = LoadAnimationFromJsonFile(L"Res/Anm/Animation.json");

	actionFile = Action::LoadFromJsonFile(L"Res/Act/ActEnemy.json");

	graphics.WaitForGpu();

	pCurOccurrence = occurrenceList;
	pEndOccurrence = occurrenceList + _countof(occurrenceList);

	sprBackground.push_back(Sprite::Sprite(anmOthers[0], XMFLOAT3(400, 300, 1.0f)));
	sprBackground[0].SetSeqIndex(0);

	sprPlayer.reserve(playerSpriteCount);
	sprPlayer.push_back(Sprite::Sprite(anmObjects[1], XMFLOAT3(400, 550, 0.4f)));
	sprPlayer[0].SetSeqIndex(0);
	sprPlayer.resize(playerSpriteCount, Sprite::Sprite(anmObjects[1], XMFLOAT3(0, -100, 0.4f)));
	for (int i = 0; i < playerShotCount; ++i) {
		freePlayerShotList.push_back(&sprPlayer[PID_PlayerShot + i]);
	}

	sprEnemy.resize(enemySpriteCount, Sprite::Sprite(anmObjects[0], XMFLOAT3(0, -100, 0.5f)));
	for (int i = 0; i < enemyCount; ++i) {
		freeEnemyList.push_back(&sprEnemy[EID_Enemy + i]);
	}
	for (int i = 0; i < enemyShotCount; ++i) {
		freeEnemyShotList.push_back(&sprEnemy[EID_EnemyShot + i]);
	}

	sprFont.reserve(256);
	static const char text[] = "00000000";
	XMFLOAT3 textPos(400 - 16 * (sizeof(text) - 1), 32, 0.1f);
	for (char c : text) {
		if (c >= ' ' && c < '`') {
			sprFont.push_back(Sprite::Sprite(anmOthers[1], textPos, 0, XMFLOAT2(1, 1), XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f)));
			sprFont.back().SetSeqIndex(c - ' ');
			textPos.x += 32.0f;
		}
	}

	time = 0.0f;

	return true;
}

/**
*
*/
bool MainGameScene::Unload()
{
	return true;
}

/**
*
*/
int MainGameScene::Update(double delta)
{
	time += delta;

	const GamePad gamepad = GetGamePad(GamePadId_1P);
	if (gamepad.buttons & GamePad::DPAD_LEFT) {
		sprPlayer[0].pos.x -= 400.0f * static_cast<float>(delta);
	} else if (gamepad.buttons & GamePad::DPAD_RIGHT) {
		sprPlayer[0].pos.x += 400.0f * static_cast<float>(delta);
	}
	if (gamepad.buttons & GamePad::DPAD_UP) {
		sprPlayer[0].pos.y -= 400.0f * static_cast<float>(delta);
	} else if (gamepad.buttons & GamePad::DPAD_DOWN) {
		sprPlayer[0].pos.y += 400.0f * static_cast<float>(delta);
	}
	sprPlayer[0].pos.x = std::max(32.0f, std::min(800.0f - 32.0f, sprPlayer[0].pos.x));
	sprPlayer[0].pos.y = std::max(32.0f, std::min(600.0f - 32.0f, sprPlayer[0].pos.y));

	if (gamepad.buttons & GamePad::A) {
		if (playerShotInterval > 0.0f) {
			playerShotInterval = std::max(playerShotInterval - static_cast<float>(delta), 0.0f);
		} else {
			static const XMVECTORF32 offset[] = {
				{ 20, -8}, {-16, -8},
				{ 24, -8}, {-20, -8},
				{ 28, -8}, {-24, -8}
			};
			for (int i = 0; i < 2 && !freePlayerShotList.empty(); ++i) {
				Sprite::Sprite* pSprite = freePlayerShotList.back();
				freePlayerShotList.pop_back();
				XMStoreFloat3(&pSprite->pos, XMVectorAdd(XMLoadFloat3(&sprPlayer[0].pos), offset[playerShotCycle]));
				pSprite->actController.SetManualMove(90, 32);
				pSprite->SetSeqIndex(1);
				playerShotCycle = (playerShotCycle + 1) % _countof(offset);
			}
			playerShotInterval = 0.125f;
		}
	} else {
		playerShotInterval = 0.0f;
	}

	for (Sprite::Sprite& sprite : sprPlayer) {
		sprite.Update(delta);
	}
	for (size_t i = PID_PlayerShot; i < PID_PlayerShot + playerShotCount; ++i) {
		if (sprPlayer[i].actController.IsDeletable()) {
			sprPlayer[i].SetActionList(nullptr);
			sprPlayer[i].pos.y = -100;
			freePlayerShotList.push_back(&sprPlayer[i]);
		}
		if (sprPlayer[i].pos.y <= -32) {
			sprPlayer[i].actController.SetManualMove(0, 0);
			freePlayerShotList.push_back(&sprPlayer[i]);
		}
	}

	while (pCurOccurrence != pEndOccurrence) {
		if (pCurOccurrence->time > time) {
			break;
		}
		formationList.push_back({ 0, pCurOccurrence->pos, pCurOccurrence->begin, pCurOccurrence->end });
		++pCurOccurrence;
	}
	for (auto itr = formationList.begin(); itr != formationList.end(); ++itr) {
		while (itr->cur != itr->end) {
			if (itr->cur->interval > itr->time) {
				break;
			}
			if (freeEnemyList.empty()) {
				++itr->cur;
				continue;
			}
			Sprite::Sprite* pSprite = freeEnemyList.back();
			freeEnemyList.pop_back();
			pSprite->SetSeqIndex(0);
			pSprite->SetActionList(actionFile->Get(0));
			pSprite->SetAction(itr->cur->actionId);
			XMStoreFloat3(&pSprite->pos, XMVectorAdd(XMLoadFloat3(&itr->pos), XMLoadFloat2(&itr->cur->offset)));
			++itr->cur;
		}
		itr->time += static_cast<float>(delta);
	}

	for (Sprite::Sprite& sprite : sprEnemy) {
		sprite.Update(delta);
	}
	for (size_t i = EID_Enemy; i < EID_Enemy + enemyCount; ++i) {
		if (sprEnemy[i].actController.IsDeletable()) {
			sprEnemy[i].SetActionList(nullptr);
			sprEnemy[i].pos.y = -100;
			freeEnemyList.push_back(&sprEnemy[i]);
		}
	}
	for (size_t i = EID_EnemyShot; i < EID_EnemyShot + enemyShotCount; ++i) {
		if (sprEnemy[i].actController.IsDeletable()) {
			sprEnemy[i].SetActionList(nullptr);
			sprEnemy[i].pos.y = -100;
			freeEnemyShotList.push_back(&sprEnemy[i]);
		}
	}

	const float brightness = static_cast<float>(std::fabs(std::fmod(time, 2.0) - 1.0));
	for (Sprite::Sprite& sprite : sprFont) {
		sprite.color.w = brightness;
		sprite.Update(delta);
	}

	for (Sprite::Sprite& sprite : sprBackground) {
		sprite.Update(delta);
	}

	return ExitCode_Continue;
}

/**
*
*/
void MainGameScene::Draw(::Scene::Graphics& graphics) const
{
	Sprite::RenderingInfo spriteRenderingInfo;
	spriteRenderingInfo.rtvHandle = graphics.GetRTVHandle();
	spriteRenderingInfo.dsvHandle = graphics.GetDSVHandle();
	spriteRenderingInfo.viewport = graphics.viewport;
	spriteRenderingInfo.scissorRect = graphics.scissorRect;
	spriteRenderingInfo.texDescHeap = graphics.csuDescriptorHeap.Get();
	spriteRenderingInfo.matViewProjection = graphics.matViewProjection;

	const PSO& pso = GetPSO(PSOType_Sprite);
	graphics.spriteRenderer.Draw(sprBackground, cellList, pso, texBackground, spriteRenderingInfo);
	graphics.spriteRenderer.Draw(sprEnemy, cellFile[1]->Get(0)->list.data(), pso, texObjects, spriteRenderingInfo);
	graphics.spriteRenderer.Draw(sprPlayer, cellPlayer->Get(0)->list.data(), pso, texObjects, spriteRenderingInfo);
	graphics.spriteRenderer.Draw(sprFont, cellFile[0]->Get(0)->list.data(), pso, texFont, spriteRenderingInfo);
}
