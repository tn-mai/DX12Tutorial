/**
* @file MainGameScene.cpp
*/
#include "MainGameScene.h"
#include "../Graphics.h"
#include "../PSO.h"
#include "../GamePad.h"
#include "../Collision.h"
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

const MainGameScene::FormationData formationD[] = {
	{ 1, 0.25f * 0,{ 0, 0 } },
	{ 1, 0.25f * 1,{ 0, 0 } },
	{ 1, 0.25f * 2,{ 0, 0 } },
	{ 1, 0.25f * 3,{ 0, 0 } },
	{ 1, 0.25f * 4,{ 0, 0 } }
};

const MainGameScene::FormationData formationE[] = {
	{ 5, 0.25f * 0,{ -81, 0 } },
	{ 5, 0.25f * 1,{  79, 0 } },
	{ 5, 0.25f * 2,{  20, 0 } },
	{ 5, 0.25f * 3,{ -10, 0 } },
	{ 5, 0.25f * 4,{  40, 0 } }
};

#define OCC_I(t, x, y, z, form) { t, {x, y, z}, form, form + _countof(form) }
#define OCC(t, x, y, z, form) OCC_I(t, x, y, z, formation##form)
#define OCC_END(t) { t, {}, nullptr, nullptr }

const MainGameScene::Occurrence occurrenceList[] = {
	OCC( 5, 600, 0, 0.5f, A),
	OCC(10, 200, 0, 0.5f, B),
	OCC(15, 300, 0, 0.5f, D),
	OCC(20, 600, 0, 0.5f, C),
	OCC(30, 600, 0, 0.5f, A),
	OCC(30, 200, 0, 0.5f, B),
	OCC(33, 200, 0, 0.5f, D),
	OCC(34, 300, 0, 0.5f, D),
	OCC(35, 400, 0, 0.5f, D),
	OCC(36.5f, 300, 0, 0.5f, E),
	OCC(38, 500, 0, 0.5f, C),
	OCC(39, 600, 0, 0.5f, C),
	OCC(40, 700, 0, 0.5f, C),
	OCC(41.5f, 600, 0, 0.5f, E),
	OCC(42.0f, 400, 0, 0.5f, E),
	OCC(45.0f, 300, 0, 0.5f, E),
	OCC(45.5f, 500, 0, 0.5f, E),
	OCC_END(60),
};

// 衝突判定データ

enum CollisionShapeId
{
	CSID_None = -1,
	CSID_Player = 0,
	CSID_PlayerShot_Normal,
	CSID_Enemy00,
	CSID_EnemyShot_Normal,
	countof_CSID
};

const Collision::Shape colShapes[countof_CSID] = {
	Collision::Shape::MakeRectangle(XMFLOAT2(-16, -16), XMFLOAT2(16, 16)),
	Collision::Shape::MakeRectangle(XMFLOAT2(-16,-64), XMFLOAT2(16, 64)),
	Collision::Shape::MakeRectangle(XMFLOAT2(-32, -32), XMFLOAT2(32, 32)),
	Collision::Shape::MakeCircle(8),
};

enum class CollisionResult
{
	Nothing, ///< 何もしない.
	FilterOut, ///< 現在の左辺側オブジェクトの衝突判定を終了する.
};

typedef std::function<CollisionResult(Sprite::Sprite& lhs, Sprite::Sprite& rhs)> CollisionSolver;

template<typename Iterator>
void DetectCollision(Iterator first0, Iterator last0, Iterator first1, Iterator last1, CollisionSolver solver)
{
	for (; first0 != last0; ++first0) {
		if (first0->GetCollisionId() < 0) {
			continue;
		}
		const Collision::Shape& shapeL = colShapes[first0->GetCollisionId()];
		const XMFLOAT2 posL(first0->pos.x, first0->pos.y);
		for (; first1 != last1; ++first1) {
			if (first1->GetCollisionId() < 0) {
				continue;
			}
			const Collision::Shape& shapeR = colShapes[first1->GetCollisionId()];
			const XMFLOAT2 posR(first1->pos.x, first1->pos.y);
			if (Collision::IsCollision(shapeL, posL, shapeR, posR)) {
				if (solver(*first0, *first1) == CollisionResult::FilterOut) {
					break;
				}
			}
		}
	}
}


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

// アニメーションID
enum EnemyAnmId
{
	EnemyAnmId_SmallFighter,
	EnemyAnmId_Shot00,
	EnemyAnmId_Shot01,
	EnemyAnmId_Shot02,
	EnemyAnmId_Destroyed,
};

enum PlayerAnmId
{
	PlayerAnmId_Ship,
	PlayerAnmId_NormalShot,
	PlayerAnmId_Destroyed,
};

// アクションID.
enum Enemy00ActId
{
	Enemy00ActId_SinCurve,
	Enemy00ActId_StraightDown,
	Enemy00ActId_VLeft,
	Enemy00ActId_VRight,
	Enemy00ActId_Destroyed,
};

} // unnamed namespace

/**
* メインゲームシーンオブジェクトを作成する.
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
* メインゲームシーンの初期化.
*/
bool MainGameScene::Load()
{
	Graphics::Graphics& graphics = Graphics::Graphics::Get();

	graphics.texMap.Begin();
	if (!graphics.texMap.LoadFromFile(texBackground, L"Res/UnknownPlanet.png")) {
		return false;
	}
	if (!graphics.texMap.LoadFromFile(texObjects, L"Res/Objects.png")) {
		return false;
	}
	if (!graphics.texMap.LoadFromFile(texFont, L"Res/TextFont.png")) {
		return false;
	}
	ID3D12CommandList* ppCommandLists[] = { graphics.texMap.End() };
	graphics.commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	cellFile[0] = Sprite::LoadFromJsonFile(L"Res/Cell/CellFont.json");
	cellFile[1] = Sprite::LoadFromJsonFile(L"Res/Cell/CellEnemy.json");
	cellPlayer = Sprite::LoadFromJsonFile(L"Res/Cell/CellPlayer.json");

	anmOthers = LoadAnimationFromJsonFile(L"Res/Anm/AnmTitle.json");
	anmObjects = LoadAnimationFromJsonFile(L"Res/Anm/Animation.json");

	actionFile = Action::LoadFromJsonFile(L"Res/Act/ActEnemy.json");

	graphics.WaitForGpu();
	graphics.texMap.ResetLoader();

	pCurOccurrence = occurrenceList;
	pEndOccurrence = occurrenceList + _countof(occurrenceList);

	sprBackground.push_back(Sprite::Sprite(anmOthers[0], XMFLOAT3(400, 300, 1.0f)));
	sprBackground[0].SetSeqIndex(0);

	sprPlayer.reserve(playerSpriteCount);
	sprPlayer.push_back(Sprite::Sprite(anmObjects[1], XMFLOAT3(400, 550, 0.4f)));
	sprPlayer[0].SetSeqIndex(0);
	sprPlayer[0].SetCollisionId(CSID_Player);
	sprPlayer.resize(playerSpriteCount, Sprite::Sprite(anmObjects[1], XMFLOAT3(0, -100, 0.4f)));
	for (int i = 0; i < playerShotCount; ++i) {
		freePlayerShotList.push_back(&sprPlayer[PID_PlayerShot + i]);
	}

	sprEnemy.resize(enemySpriteCount, Sprite::Sprite(anmObjects[0], XMFLOAT3(0, -100, 0.5f)));
	for (int i = 0; i < enemyCount; ++i) {
		sprEnemy[EID_Enemy + i].SetCollisionId(CSID_None);
		freeEnemyList.push_back(&sprEnemy[EID_Enemy + i]);
	}
	for (int i = 0; i < enemyShotCount; ++i) {
		sprEnemy[EID_EnemyShot + i].SetCollisionId(CSID_None);
		freeEnemyShotList.push_back(&sprEnemy[EID_EnemyShot + i]);
	}

	sprFont.reserve(256);
	static const char text[] = "00000000";
	XMFLOAT3 textPos(400 - (_countof(text) - 2) * 16, 32, 0.1f);
	for (const char c : text) {
		if (c >= ' ' && c < '`') {
			sprFont.push_back(Sprite::Sprite(anmOthers[1], textPos, 0, XMFLOAT2(1, 1), XMFLOAT4(0.5f, 1.0f, 0.5f, 0.5f)));
			sprFont.back().SetSeqIndex(c - ' ');
			textPos.x += 32.0f;
		}
	}

	Audio::Engine& audio = Audio::Engine::Get();
	seBomb = audio.Prepare(L"Res/SE/Bomb.wav");
	sePlayerShot = audio.Prepare(L"Res/SE/PlayerShot.wav");
	bgm = Audio::Engine::Get().PrepareStream(L"Res/SE/MainGame.xwm");
	bgm->Play(Audio::Flag_Loop);

	time = 0.0f;

	return true;
}

/**
* メインゲームシーンの終了処理.
*/
bool MainGameScene::Unload()
{
	bgm->Stop();
	bgm.reset();
	seBomb.reset();
	sePlayerShot.reset();
	return true;
}

/**
* プレイヤー状態の更新.
*/
void MainGameScene::UpdatePlayer(double delta)
{
	const GamePad gamepad = GetGamePad(GamePadId_1P);

	if (sprPlayer[0].animeController.GetSeqIndex() != PlayerAnmId_Destroyed) {
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
					{ 20, -8 },{ -16, -8 },
					{ 24, -8 },{ -20, -8 },
					{ 28, -8 },{ -24, -8 }
				};
				for (int i = 0; i < 2 && !freePlayerShotList.empty(); ++i) {
					Sprite::Sprite* pSprite = freePlayerShotList.back();
					freePlayerShotList.pop_back();
					XMStoreFloat3(&pSprite->pos, XMVectorAdd(XMLoadFloat3(&sprPlayer[0].pos), offset[playerShotCycle]));
					pSprite->actController.SetManualMove(90, 960);
					pSprite->SetSeqIndex(PlayerAnmId_NormalShot);
					pSprite->SetCollisionId(CSID_PlayerShot_Normal);
					playerShotCycle = (playerShotCycle + 1) % _countof(offset);
				}
				sePlayerShot->Play();
				playerShotInterval = 0.125f;
			}
		} else {
			playerShotInterval = 0.0f;
		}
	}

	for (Sprite::Sprite& sprite : sprPlayer) {
		sprite.Update(delta);
	}
	for (size_t i = PID_PlayerShot; i < PID_PlayerShot + playerShotCount; ++i) {
		if (sprPlayer[i].GetCollisionId() < 0) {
			continue;
		}
		if (sprPlayer[i].actController.IsDeletable() || (sprPlayer[i].pos.y <= -32)) {
			sprPlayer[i].pos.y = -32;
			sprPlayer[i].SetActionList(nullptr);
			sprPlayer[i].SetCollisionId(CSID_None);
			freePlayerShotList.push_back(&sprPlayer[i]);
		}
	}
}

/**
* 敵の生成.
*/
void MainGameScene::GenerateEnemy(double delta)
{
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
			pSprite->SetSeqIndex(EnemyAnmId_SmallFighter);
			pSprite->SetActionList(actionFile->Get(0));
			pSprite->SetAction(itr->cur->actionId);
			pSprite->SetCollisionId(CSID_Enemy00);
			XMStoreFloat3(&pSprite->pos, XMVectorAdd(XMLoadFloat3(&itr->pos), XMLoadFloat2(&itr->cur->offset)));
			++itr->cur;
		}
		itr->time += static_cast<float>(delta);
	}
}

/**
* 敵の更新.
*/
void MainGameScene::UpdateEnemy(double delta)
{
	for (Sprite::Sprite& sprite : sprEnemy) {
		sprite.Update(delta);
	}
	for (size_t i = EID_Enemy; i < EID_Enemy + enemyCount; ++i) {
		Sprite::Sprite& p = sprEnemy[i];
		if (p.GetCollisionId() == CSID_None) {
			continue;
		}
		if (p.actController.IsDeletable()) {
			p.SetActionList(nullptr);
			p.SetCollisionId(CSID_None);
			p.pos.y = -100;
			freeEnemyList.push_back(&p);
		}
	}
	for (size_t i = EID_EnemyShot; i < EID_EnemyShot + enemyShotCount; ++i) {
		Sprite::Sprite& p = sprEnemy[i];
		if (p.GetCollisionId() == CSID_None) {
			continue;
		}
		if (p.actController.IsDeletable() ||
			p.pos.x < -32 || p.pos.x >= 832 ||
			p.pos.y < -32 || p.pos.y >= 632) {
			p.SetActionList(nullptr);
			p.SetCollisionId(CSID_None);
			p.pos.y = -100;
			freeEnemyShotList.push_back(&p);
		}
	}
}

/**
* スコア表示の更新.
*/
void MainGameScene::UpdateScore()
{
	char text[] = "00000000";
	snprintf(text, _countof(text), "%08d", score);
	int i = 0;
	bool active = false;
	for (const char c : text) {
		if (c >= ' ' && c < '`') {
			if (c > '0') {
				active = true;
			}
			sprFont[i].color.w = active ? 1.0f : 0.5f;
			sprFont[i++].SetSeqIndex(c - ' ');
		}
	}
#if 0
	const float brightness = static_cast<float>(std::fabs(std::fmod(time, 2.0) - 1.0));
	for (Sprite::Sprite& sprite : sprFont) {
		sprite.color.w = brightness;
		sprite.Update(delta);
	}
#endif
}

/**
* 衝突の解決.
*/
void MainGameScene::SolveCollision()
{
	DetectCollision(
		sprPlayer.begin() + PID_PlayerShot, sprPlayer.end(),
		sprEnemy.begin() + EID_Enemy, sprEnemy.begin() + enemyCount,
		[this](Sprite::Sprite& a, Sprite::Sprite& b) {
		if (a.pos.y <= -32) {
			a.SetCollisionId(CSID_None);
			return CollisionResult::FilterOut;
		}
		a.pos.y = -32;
		a.actController.SetManualMove(0, 0);
		a.SetCollisionId(CSID_None);
		a.SetActionList(nullptr);
		freePlayerShotList.push_back(&a);
		if (b.animeController.GetSeqIndex() == EnemyAnmId_SmallFighter) {
			b.animeController.SetSeqIndex(EnemyAnmId_Destroyed);
			b.SetAction(Enemy00ActId_Destroyed);
			b.SetCollisionId(CSID_None);
			score += 100;
			seBomb->Play();
			VibrateGamePad(GamePadId_1P, 1);
		}
		return CollisionResult::FilterOut;
	}
	);
	DetectCollision(
		sprPlayer.begin() + PID_Player, sprPlayer.begin() + playerCount,
		sprEnemy.begin(), sprEnemy.end(),
		[this](Sprite::Sprite& a, Sprite::Sprite& b) {
		a.animeController.SetSeqIndex(PlayerAnmId_Destroyed);
		a.SetCollisionId(CSID_None);
		if (b.animeController.GetSeqIndex() == EnemyAnmId_SmallFighter) {
			b.animeController.SetSeqIndex(EnemyAnmId_Destroyed);
			b.SetAction(Enemy00ActId_Destroyed);
			b.SetCollisionId(CSID_None);
			score += 100;
			VibrateGamePad(GamePadId_1P, 0);
		} else {
			b.pos.y = -32;
			b.actController.SetManualMove(0, 0);
		}
		seBomb->Play();
		return CollisionResult::FilterOut;
	}
	);
}

/**
* ゲーム状態の更新.
*/
int MainGameScene::Update(double delta)
{
	time += delta;

	UpdatePlayer(delta);
	UpdateEnemy(delta);
	GenerateEnemy(delta);

	SolveCollision();

	UpdateScore();

	for (Sprite::Sprite& sprite : sprBackground) {
		sprite.Update(delta);
	}

	const GamePad gamepad = GetGamePad(GamePadId_1P);
	static const uint32_t endingKey = GamePad::A | GamePad::START;
	if (pCurOccurrence == pEndOccurrence) {
		return ExitCode_Ending;
	}
	if (sprPlayer[0].animeController.GetSeqIndex() == PlayerAnmId_Destroyed && sprPlayer[0].animeController.IsFinished()) {
		return ExitCode_GameOver;
	}
	if (gamepad.buttonDown & GamePad::START) {
		return ExitCode_Pause;
	}
	return ExitCode_Continue;
}

/**
* メインゲームシーンの描画.
*/
void MainGameScene::Draw(Graphics::Graphics& graphics) const
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
