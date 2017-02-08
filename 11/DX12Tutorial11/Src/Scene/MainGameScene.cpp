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

enum EnemyType
{
	EnemyType_None = -1,
	EnemyType_Winp,
	EnemyType_3Way,
	EnemyType_Middle,
	EnemyType_Boss1st,
	EnemyType_BgmStop,
	EnemyType_BgmBoss,
};

struct MainGameScene::FormationData {
	EnemyType type;
	uint32_t actionId;
	float interval;
	XMFLOAT2 offset;
};

struct MainGameScene::Occurrence {
	float time;
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
	{ EnemyType_Winp, 2, 0.25f * 0, { 0, 0 } },
	{ EnemyType_Winp, 2, 0.25f * 1, { 0, 0 } },
	{ EnemyType_Winp, 2, 0.25f * 2, { 0, 0 } },
	{ EnemyType_Winp, 2, 0.25f * 3, { 0, 0 } },
	{ EnemyType_Winp, 2, 0.25f * 4, { 0, 0 } }
};

const MainGameScene::FormationData formationB[] = {
	{ EnemyType_Winp, 3, 0.25f * 0,{ 0, 0 } },
	{ EnemyType_Winp, 3, 0.25f * 1,{ 0, 0 } },
	{ EnemyType_Winp, 3, 0.25f * 2,{ 0, 0 } },
	{ EnemyType_Winp, 3, 0.25f * 3,{ 0, 0 } },
	{ EnemyType_Winp, 3, 0.25f * 4,{ 0, 0 } }
};

const MainGameScene::FormationData formationC[] = {
	{ EnemyType_Winp, 0, 0.25f * 0,{ 0, 0 } },
	{ EnemyType_Winp, 0, 0.25f * 1,{ 0, 0 } },
	{ EnemyType_Winp, 0, 0.25f * 2,{ 0, 0 } },
	{ EnemyType_Winp, 0, 0.25f * 3,{ 0, 0 } },
	{ EnemyType_Winp, 0, 0.25f * 4,{ 0, 0 } }
};

const MainGameScene::FormationData formationD[] = {
	{ EnemyType_Winp, 1, 0.25f * 0,{ 0, 0 } },
	{ EnemyType_Winp, 1, 0.25f * 1,{ 0, 0 } },
	{ EnemyType_Winp, 1, 0.25f * 2,{ 0, 0 } },
	{ EnemyType_Winp, 1, 0.25f * 3,{ 0, 0 } },
	{ EnemyType_Winp, 1, 0.25f * 4,{ 0, 0 } }
};

const MainGameScene::FormationData formationE[] = {
	{ EnemyType_Winp, 5, 0.25f * 0,{ -81, 0 } },
	{ EnemyType_Winp, 5, 0.25f * 1,{ 79, 0 } },
	{ EnemyType_Winp, 5, 0.25f * 2,{ 20, 0 } },
	{ EnemyType_Winp, 5, 0.25f * 3,{ -10, 0 } },
	{ EnemyType_Winp, 5, 0.25f * 4,{  40, 0 } }
};

const MainGameScene::FormationData formation3Way[] = {
	{ EnemyType_3Way, 0, 0,{ 0, 0 } },
};

const MainGameScene::FormationData formationMiddleL[] = {
	{ EnemyType_Middle, 0, 0,{ 0, 0 } },
};

const MainGameScene::FormationData formationMiddleR[] = {
	{ EnemyType_Middle, 1, 0,{ 0, 0 } },
};

const MainGameScene::FormationData formationMiddle2[] = {
	{ EnemyType_Middle, 3, 0,{ 0, 0 } },
};

const MainGameScene::FormationData formationBoss1st[] = {
	{ EnemyType_Boss1st, 0, 0,{ 0, -32 } },
};

const MainGameScene::FormationData formationBgmStop[] = {
	{ EnemyType_BgmStop },
};

const MainGameScene::FormationData formationBgmBoss[] = {
	{ EnemyType_BgmBoss },
};

#define OCC_I(t, x, y, z, form) { t, {x, y, z}, form, form + _countof(form) }
#define OCC(t, x, y, z, form) OCC_I(t, x, y, z, formation##form)
#define OCC_END(t) { t, {}, nullptr, nullptr }

const MainGameScene::Occurrence occurrenceList[] = {
#if 0 // ボス動作テスト用.
	OCC( 5, 400, 0, 0.5f, Boss1st),
#elif 0 // 中型機動作テスト用.
	OCC( 5, 400, 0, 0.5f, Middle2),
#else
	OCC( 5, 600, 0, 0.5f, A),
	OCC(10, 200, 0, 0.5f, B),
	OCC(15, 300, 0, 0.5f, D),
	OCC(20, 600, 0, 0.5f, C),
	OCC(24, 250, 0, 0.5f, 3Way),
	OCC(25, 550, 0, 0.5f, 3Way),
	OCC(26, 400, 0, 0.5f, 3Way),
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
	OCC(47, 250, 0, 0.5f, 3Way),
	OCC(48, 550, 0, 0.5f, 3Way),
	OCC(49, 400, 0, 0.5f, 3Way),
	OCC(51, 400, 0, 0.5f, MiddleL),
	OCC(53, 200, 0, 0.5f, 3Way),
	OCC(54, 350, 0, 0.5f, 3Way),
	OCC(55, 500, 0, 0.5f, 3Way),
	OCC(57, 600, 0, 0.5f, 3Way),
	OCC(58, 450, 0, 0.5f, 3Way),
	OCC(59, 300, 0, 0.5f, 3Way),
	OCC(63, 600, 0, 0.5f, C),
	OCC(64, 300, 0, 0.5f, D),
	OCC(67, 600, 0, 0.5f, MiddleR),
	OCC(72, 200, 0, 0.5f, MiddleL),
	OCC(77, 400, 0, 0.5f, MiddleR),
	OCC(82, 600, 0, 0.5f, 3Way),
	OCC(82, 300, 0, 0.5f, 3Way),
	OCC(85, 450, 0, 0.5f, 3Way),
	OCC(88, 600, 0, 0.5f, MiddleR),
	OCC(88, 200, 0, 0.5f, MiddleL),
	OCC(90, 0, 0, 0, BgmStop),
	OCC(92, 0, 0, 0, BgmBoss),
	OCC(100, 400, 0, 0.5f, Boss1st),
#endif
	OCC(163, 0, 0, 0, BgmStop),
	OCC_END(165),
};

// 衝突判定データ

enum CollisionShapeId
{
	CSID_None = -1,
	CSID_Player = 0,
	CSID_PlayerShot_Normal,
	CSID_Enemy00,
	CSID_Enemy3Way,
	CSID_EnemyMiddle,
	CSID_EnemyBoss1st,
	CSID_EnemyShot_Normal,
	countof_CSID
};

const Collision::Shape colShapes[countof_CSID] = {
	Collision::Shape::MakeRectangle(XMFLOAT2(-16, -8), XMFLOAT2(16, 24)),
	Collision::Shape::MakeRectangle(XMFLOAT2(-8,-32), XMFLOAT2(8, 32)),
	Collision::Shape::MakeRectangle(XMFLOAT2(-24, -24), XMFLOAT2(24, 24)),
	Collision::Shape::MakeRectangle(XMFLOAT2(-24, -24), XMFLOAT2(24, 32)),
	Collision::Shape::MakeRectangle(XMFLOAT2(-48, -48), XMFLOAT2(48, 48)),
	Collision::Shape::MakeRectangle(XMFLOAT2(-240, -64), XMFLOAT2(240, 96)),
	Collision::Shape::MakeCircle(2),
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
	for (Iterator itr0 = first0; itr0 != last0; ++itr0) {
		if (itr0->GetCollisionId() < 0) {
			continue;
		}
		const Collision::Shape& shapeL = colShapes[itr0->GetCollisionId()];
		const XMFLOAT2 posL(itr0->pos.x, itr0->pos.y);
		for (Iterator itr1 = first1; itr1 != last1; ++itr1) {
			if (itr1->GetCollisionId() < 0) {
				continue;
			}
			const Collision::Shape& shapeR = colShapes[itr1->GetCollisionId()];
			const XMFLOAT2 posR(itr1->pos.x, itr1->pos.y);
			if (Collision::IsCollision(shapeL, posL, shapeR, posR)) {
				if (solver(*itr0, *itr1) == CollisionResult::FilterOut) {
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
	EnemyAnmId_3Way_FrontMove,
	EnemyAnmId_3Way_BackMove,
	EnemyAnmId_3Way_Attack,
	EnemyAnmId_MiddleFighter,
	EnemyAnmId_Boss1st,
};

enum PlayerAnmId
{
	PlayerAnmId_Ship,
	PlayerAnmId_NormalShot,
	PlayerAnmId_Destroyed,
};

// アクションリストID.
enum EnemyActListId
{
	EnemyActListId_Winp,
	EnemyActListId_3Way,
	EnemyActListId_Middle,
	EnemyActListId_Boss1st,
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
bool MainGameScene::Load(::Scene::Context& context)
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
	clearTime = (pEndOccurrence - 1)->time;

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
	seHit = audio.Prepare(L"Res/SE/Hit.wav");
	seBombBoss = audio.Prepare(L"Res/SE/BombBoss.wav");
	sePlayerShot = audio.Prepare(L"Res/SE/PlayerShot.wav");
	bgm = Audio::Engine::Get().PrepareStream(L"Res/SE/MainGame.xwm");
	bgm->SetVolume(2.0f);
	bgm->Play(Audio::Flag_Loop);

	time = 0.0f;
	context.score = 0;

	return true;
}

/**
* メインゲームシーンの終了処理.
*/
bool MainGameScene::Unload(::Scene::Context&)
{
	bgm->Stop();
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
					pSprite->actController.SetManualMove(90, 32 * 60);
					pSprite->SetSeqIndex(PlayerAnmId_NormalShot);
					pSprite->SetCollisionId(CSID_PlayerShot_Normal);
					playerShotCycle = (playerShotCycle + 1) % _countof(offset);
				}
				sePlayerShot->Play();
				playerShotInterval = 0.125f / 1.0f;
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
* 敵弾生成器.
*/
class EnemyShotGenerator
{
public:
	EnemyShotGenerator() = default;
	EnemyShotGenerator(const Sprite::Sprite& p, std::vector<Sprite::Sprite*>& fl, uint32_t n = 1, float i = 2.0f) :
		player(p), freeList(fl), stock(n), timer(0), interval(i), speed(-1)
	{}
	void operator()(float delta, Sprite::Sprite* spr, float s, float r) {
		if (speed < 0) {
			speed = s;
		}
		if (stock <= 0) {
			return;
		}
		if (timer > delta) {
			timer -= delta;
			return;
		}
		timer = 0.0f;
		if (freeList.empty()) {
			return;
		}
		--stock;
		timer = interval;

		Sprite::Sprite* pSprite = freeList.back();
		freeList.pop_back();
		pSprite->pos = spr->pos;
		XMVECTORF32 vec, angle;
		vec.v = XMVector2Normalize(XMVectorSubtract(XMLoadFloat3(&player.pos), XMLoadFloat3(&spr->pos)));
		angle.v = XMVectorACos(XMVectorSwizzle<1, 1, 1, 1>(vec));
		pSprite->rotation = vec.f[0] < 0.0f ? angle.f[0] : -angle.f[0];
		vec.v = XMVectorMultiply(vec, XMVectorSwizzle<0, 0, 0, 0>(XMLoadFloat(&speed)));
		XMFLOAT2 move;
		XMStoreFloat2(&move, vec);
		pSprite->actController.SetManualMove(move);
		pSprite->SetSeqIndex(EnemyAnmId_Shot00);
		pSprite->SetCollisionId(CSID_EnemyShot_Normal);
	}
private:
	const Sprite::Sprite& player;
	std::vector<Sprite::Sprite*>& freeList;
	uint32_t stock;
	float timer;
	float interval;
	float speed;
};

/**
* 3 way 敵弾生成器.
*/
class Enemy3WayShotGenerator
{
public:
	Enemy3WayShotGenerator() = default;
	explicit Enemy3WayShotGenerator(std::vector<Sprite::Sprite*>& fl) : freeList(fl), isActed(false) {}
	void operator()(float delta, Sprite::Sprite* spr, float s, float r) {
		if (isActed) {
			return;
		}
		static const struct {
			float dir;
			XMVECTORF32 pos;
		} info[] = {
			{ 270, { 0, 32 } },
			{ 270 - 15, { -8, 32 } },
			{ 270 + 15, { 8, 32 } },
		};
		for (auto itr = std::begin(info); itr != std::end(info); ++itr) {
			if (freeList.empty()) {
				break;
			}
			Sprite::Sprite* pSprite = freeList.back();
			freeList.pop_back();
			XMStoreFloat3(&pSprite->pos, XMVectorAdd(XMLoadFloat3(&spr->pos), itr->pos));
			pSprite->rotation = (270 - itr->dir) * 3.1415926f / 180.0f;
			pSprite->actController.SetManualMove(itr->dir, s);
			pSprite->SetSeqIndex(EnemyAnmId_Shot00);
			pSprite->SetCollisionId(CSID_EnemyShot_Normal);
		}
		isActed = true;
	}
private:
	std::vector<Sprite::Sprite*>& freeList;
	bool isActed;
};

/**
* 敵弾生成器.
*/
class EnemyBoss1stShotGenerator
{
public:
	EnemyBoss1stShotGenerator() = default;
	EnemyBoss1stShotGenerator(const Sprite::Sprite& p, std::vector<Sprite::Sprite*>& fl, uint32_t n = 1, float i = 2.0f) :
		player(p), freeList(fl), interval(i), speed(-1)
	{}
	void operator()(float delta, Sprite::Sprite* spr, float s, float r) {
		if (speed < 0) {
			speed = s;
			timer[0] = timer[1] = timer[2] = 0;
			stock[0] = 3;
			stock[1] = 16;
			stock[2] = 3;
		}
		Sequence0(delta, spr);
		if (spr->hp < 300) {
			Sequence2(delta, spr);
		}
		if (spr->hp < 150) {
			Sequence1(delta, spr);
		}
	}

	// 中央の二門の砲からの5連ショット.
	void Sequence0(float delta, Sprite::Sprite* spr)
	{
		if (timer[0] > delta) {
			timer[0] -= delta;
			return;
		}
		timer[0] = 0.0f;

		static const XMVECTORF32 pos[] = {
			{ -24, 88 }, { 24, 88 }
		};
		for (int i = 0; i < 2; ++i) {
			if (freeList.empty()) {
				return;
			}
			Sprite::Sprite* pSprite = freeList.back();
			freeList.pop_back();
			XMStoreFloat3(&pSprite->pos, XMVectorAdd(XMLoadFloat3(&spr->pos), pos[i]));
			pSprite->rotation = 0;
			pSprite->actController.SetManualMove(270, 600);
			pSprite->SetSeqIndex(EnemyAnmId_Shot01);
			pSprite->SetCollisionId(CSID_EnemyShot_Normal);
		}
		if (--stock[0] <= 0) {
			stock[0] = 3;
			timer[0] += 2.0f;
		} else {
			timer[0] += 1.0f / 8.0f;
		}
	}

	// 主翼上面砲台からの全方位ショット.
	void Sequence1(float delta, Sprite::Sprite* spr)
	{
		if (timer[1] > delta) {
			timer[1] -= delta;
			return;
		}
		timer[1] = 0.0f;
		static const XMVECTORF32 pos[] = {
			{ -88, -24 },{ 88, -24 }
		};
		for (int i = 0; i < 2; ++i) {
			if (freeList.empty()) {
				return;
			}
			Sprite::Sprite* pSprite = freeList.back();
			freeList.pop_back();
			XMStoreFloat3(&pSprite->pos, XMVectorAdd(XMLoadFloat3(&spr->pos), pos[i]));
			if (i == 0) {
				pSprite->actController.SetManualMove((128 - stock[1] - 4) * 360.0f / 16.0f, 200);
			} else {
				pSprite->actController.SetManualMove((stock[1] - 4) * 360.0f / 16.0f, 200);
			}
			pSprite->rotation = 0;
			pSprite->SetSeqIndex(EnemyAnmId_Shot02);
			pSprite->SetCollisionId(CSID_EnemyShot_Normal);
		}
		if (--stock[1] <= 0) {
			stock[1] = 64;
			timer[1] += 4.0f;
		} else {
			timer[1] += 1.0f / 8.0f;
		}
	}

	// 主翼縁の三門の砲からの狙いショット.
	void Sequence2(float delta, Sprite::Sprite* spr)
	{
		if (timer[2] > delta) {
			timer[2] -= delta;
			return;
		}
		timer[2] = 0.0f;

		static const XMVECTORF32 pos[][3] = {
			{ { -136, 56 }, {-160, 64 }, { -184, 72 } },
			{ { 136, 56 },{ 160, 64 },{ 184, 72 } },
		};
		for (int i = 0; i < 2; ++i) {
			if (freeList.empty()) {
				return;
			}
			Sprite::Sprite* pSprite = freeList.back();
			freeList.pop_back();
			XMStoreFloat3(&pSprite->pos, XMVectorAdd(XMLoadFloat3(&spr->pos), pos[i][(stock[2] % 3)]));

			XMVECTORF32 vec, angle;
			vec.v = XMVector2Normalize(XMVectorSubtract(XMLoadFloat3(&player.pos), XMLoadFloat3(&pSprite->pos)));
			angle.v = XMVectorACos(XMVectorSwizzle<1, 1, 1, 1>(vec));
			pSprite->rotation = vec.f[0] < 0.0f ? angle.f[0] : -angle.f[0];
			vec.v = XMVectorMultiply(vec, XMVectorSwizzle<0, 0, 0, 0>(XMLoadFloat(&speed)));
			XMFLOAT2 move;
			XMStoreFloat2(&move, vec);
			pSprite->actController.SetManualMove(move);
			pSprite->SetSeqIndex(EnemyAnmId_Shot00);
			pSprite->SetCollisionId(CSID_EnemyShot_Normal);
		}
		if (--stock[2] <= 0) {
			stock[2] = 3;
			timer[2] += 1.0f;
		} else {
			timer[2] += 1.0f / 8.0f;
		}
	}

private:
	const Sprite::Sprite& player;
	std::vector<Sprite::Sprite*>& freeList;
	int32_t stock[3];
	float timer[3];
	float interval;
	float speed;
};

/**
* 敵の生成.
*/
void MainGameScene::GenerateEnemy(double delta)
{
	struct local {
		static Sprite::Sprite* GetSprite(std::vector<Sprite::Sprite*>& freeList, const MainGameScene::Formation& data) {
			if (freeList.empty()) {
				return nullptr;
			}
			Sprite::Sprite* p = freeList.back();
			freeList.pop_back();
			XMStoreFloat3(&p->pos, XMVectorAdd(XMLoadFloat3(&data.pos), XMLoadFloat2(&data.cur->offset)));
			return p;
		}
	};
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
			switch (itr->cur->type) {
			case EnemyType_Winp:
				if (Sprite::Sprite* pSprite = local::GetSprite(freeEnemyList, *itr)) {
					pSprite->SetSeqIndex(EnemyAnmId_SmallFighter);
					pSprite->SetActionList(actionFile->Get(EnemyActListId_Winp));
					pSprite->actController.SetGenerator(EnemyShotGenerator(sprPlayer[0], freeEnemyShotList));
					pSprite->SetAction(itr->cur->actionId);
					pSprite->SetCollisionId(CSID_Enemy00);
					pSprite->hp = 1;
				}
				break;
			case EnemyType_3Way:
				if (Sprite::Sprite* pSprite = local::GetSprite(freeEnemyList, *itr)) {
					pSprite->SetActionList(actionFile->Get(EnemyActListId_3Way));
					pSprite->SetAction(itr->cur->actionId);
					pSprite->actController.SetGenerator(Enemy3WayShotGenerator(freeEnemyShotList));
					pSprite->SetCollisionId(CSID_Enemy3Way);
					pSprite->hp = 3;
				}
				break;
			case EnemyType_Middle:
				if (Sprite::Sprite* pSprite = local::GetSprite(freeEnemyList, *itr)) {
					pSprite->SetActionList(actionFile->Get(EnemyActListId_Middle));
					pSprite->SetAction(itr->cur->actionId);
					pSprite->actController.SetGenerator(EnemyShotGenerator(sprPlayer[0], freeEnemyShotList, 5));
					pSprite->SetCollisionId(CSID_EnemyMiddle);
					pSprite->hp = 40;
				}
				break;
			case EnemyType_Boss1st:
				if (Sprite::Sprite* pSprite = local::GetSprite(freeEnemyList, *itr)) {
					pSprite->SetActionList(actionFile->Get(EnemyActListId_Boss1st));
					pSprite->SetAction(itr->cur->actionId);
					pSprite->actController.SetGenerator(EnemyBoss1stShotGenerator(sprPlayer[0], freeEnemyShotList, 500, 0.25f));
					pSprite->SetCollisionId(CSID_EnemyBoss1st);
					pSprite->hp = 400;
				}
				break;
			case EnemyType_BgmStop:
				bgmFadeOut = true;
				break;
			case EnemyType_BgmBoss:
				bgmFadeOut = false;
				bgm->Stop();
				bgm = Audio::Engine::Get().PrepareStream(L"Res/SE/Boss1st.xwm");
				OutputDebugStringA(bgm ? "Boss1st: OK\n" : "Boss1st: NG\n");
				bgm->SetVolume(2.0f);
				bgm->Play(Audio::Flag_Loop);
				break;
			}
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
			continue;
		}
		p.color = XMFLOAT4(1, 1, 1, 1);
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
void MainGameScene::UpdateScore(uint32_t score)
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
void MainGameScene::SolveCollision(::Scene::Context& context)
{
	DetectCollision(
		sprPlayer.begin() + PID_PlayerShot, sprPlayer.end(),
		sprEnemy.begin() + EID_Enemy, sprEnemy.begin() + enemyCount,
		[this, &context](Sprite::Sprite& a, Sprite::Sprite& b) {
		if (a.pos.y <= -32) {
			a.actController.SetManualMove(0, 0);
			a.SetCollisionId(CSID_None);
			a.SetActionList(nullptr);
			freePlayerShotList.push_back(&a);
			return CollisionResult::FilterOut;
		}
		a.pos.y = -32;
		a.actController.SetManualMove(0, 0);
		a.SetCollisionId(CSID_None);
		a.SetActionList(nullptr);
		freePlayerShotList.push_back(&a);
		if (--b.hp > 0) {
			context.score += 10;
			b.color = XMFLOAT4(1, 0, 0, 1);
			seHit->Play();
			return CollisionResult::FilterOut;
		}
		switch (b.animeController.GetSeqIndex()) {
		case EnemyAnmId_SmallFighter:
			b.animeController.SetSeqIndex(EnemyAnmId_Destroyed);
			b.actController.SetGenerator(nullptr);
			b.SetCollisionId(CSID_None);
			b.SetAction(Enemy00ActId_Destroyed);
			context.score += 100;
			seBomb->Play();
			VibrateGamePad(GamePadId_1P, 1);
			break;
		case EnemyAnmId_3Way_FrontMove:
		case EnemyAnmId_3Way_BackMove:
		case EnemyAnmId_3Way_Attack:
			b.animeController.SetSeqIndex(EnemyAnmId_Destroyed);
			b.actController.SetGenerator(nullptr);
			b.SetCollisionId(CSID_None);
			b.SetAction(1);
			context.score += 200;
			seBomb->Play();
			VibrateGamePad(GamePadId_1P, 1);
			break;
		case EnemyAnmId_MiddleFighter:
			b.animeController.SetSeqIndex(EnemyAnmId_Destroyed);
			b.actController.SetGenerator(nullptr);
			b.scale = XMFLOAT2(2, 2);
			b.SetCollisionId(CSID_None);
			b.SetAction(2);
			context.score += 800;
			seBomb->Play();
			VibrateGamePad(GamePadId_1P, 1);
			break;
		case EnemyAnmId_Boss1st:
			b.animeController.SetSeqIndex(EnemyAnmId_Destroyed);
			b.actController.SetGenerator(nullptr);
			b.SetCollisionId(CSID_None);
			b.SetActionList(nullptr);
			context.score += 5000;
			seBombBoss->Play();
			VibrateGamePad(GamePadId_1P, 2);
			clearTime = time + 5.0f;
			bgmFadeOut = true;
			{
				static const XMVECTORF32 pos[] = { { 0, 0 }, {-100, 50}, { 100, 50 } };
				for (int i = 0; i < _countof(pos); ++i) {
					if (freeEnemyList.empty()) {
						break;
					}
					Sprite::Sprite* p = freeEnemyList.back();
					freeEnemyList.pop_back();
					XMStoreFloat3(&p->pos, XMVectorAdd(pos[i], XMLoadFloat3(&b.pos)));
					p->animeController.SetSeqIndex(EnemyAnmId_Destroyed);
					p->scale = XMFLOAT2(4, 4);
					p->SetActionList(nullptr);
				}
			}
			break;
		}
		return CollisionResult::FilterOut;
	}
	);
	DetectCollision(
		sprPlayer.begin() + PID_Player, sprPlayer.begin() + playerCount,
		sprEnemy.begin(), sprEnemy.end(),
		[this, &context](Sprite::Sprite& a, Sprite::Sprite& b) {
		a.animeController.SetSeqIndex(PlayerAnmId_Destroyed);
		a.SetCollisionId(CSID_None);
		if (b.animeController.GetSeqIndex() == EnemyAnmId_SmallFighter) {
			b.animeController.SetSeqIndex(EnemyAnmId_Destroyed);
			b.SetAction(Enemy00ActId_Destroyed);
			b.SetCollisionId(CSID_None);
			context.score += 100;
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
int MainGameScene::Update(::Scene::Context& context, double delta)
{
	time += delta;

	if (bgmFadeOut) {
		bgmVolume -= static_cast<float>(delta) * 0.5f;
		if (bgmVolume < 0) {
			bgmVolume = 0;
			bgmFadeOut = false;
		}
		bgm->SetVolume(bgmVolume);
	}

	UpdatePlayer(delta);
	UpdateEnemy(delta);
	GenerateEnemy(delta);

	SolveCollision(context);

	UpdateScore(context.score);

	for (Sprite::Sprite& sprite : sprBackground) {
		sprite.Update(delta);
	}

	const GamePad gamepad = GetGamePad(GamePadId_1P);
	static const uint32_t endingKey = GamePad::A | GamePad::START;
	if (time >= clearTime) {
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
