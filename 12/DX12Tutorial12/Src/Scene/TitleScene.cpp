/**
* @file TitleScene.cpp
*/
#include "TitleScene.h"
#include "../Graphics.h"
#include "../PSO.h"
#include "../GamePad.h"
#include "../Animation.h"
#include <DirectXMath.h>
#include <algorithm>

using namespace DirectX;

/**
* スプライト用セルデータ.
*/
const Sprite::Cell bgCellList[] = {
	{ XMFLOAT2(0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT2(800, 600) },
};

const Sprite::Cell charCellList[] = {
	{ XMFLOAT2(0/32.0f, 30/32.0f), XMFLOAT2(2/32.0f, 2/32.0f), XMFLOAT2(64, 64) },
	{ XMFLOAT2(2/32.0f, 30/32.0f), XMFLOAT2(2/32.0f, 2/32.0f), XMFLOAT2(64, 64) },
	{ XMFLOAT2(4/32.0f, 30/32.0f), XMFLOAT2(2/32.0f, 2/32.0f), XMFLOAT2(64, 64) },

	{ XMFLOAT2(0/32.0f, 28/32.0f), XMFLOAT2(1/32.0f, 2/32.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(0/32.0f, 28/32.0f), XMFLOAT2(1/32.0f, 2/32.0f), XMFLOAT2(16, 64) },

	{ XMFLOAT2(0/32.0f, 0/32.0f), XMFLOAT2(2/32.0f, 2/32.0f), XMFLOAT2(64, 64) },
	{ XMFLOAT2(2/32.0f, 0/32.0f), XMFLOAT2(2/32.0f, 2/32.0f), XMFLOAT2(64, 64) },
	{ XMFLOAT2(4/32.0f, 0/32.0f), XMFLOAT2(2/32.0f, 2/32.0f), XMFLOAT2(64, 64) },
	{ XMFLOAT2(6/32.0f, 0/32.0f), XMFLOAT2(2/32.0f, 2/32.0f), XMFLOAT2(64, 64) },

	{ XMFLOAT2(0/32.0f, 2/32.0f), XMFLOAT2(1/32.0f, 1/32.0f), XMFLOAT2(32, 32) },
	{ XMFLOAT2(0/32.0f, 2/32.0f), XMFLOAT2(1/32.0f, 1/32.0f), XMFLOAT2(16, 32) },

	{ XMFLOAT2(0/32.0f, 4/32.0f), XMFLOAT2(2/32.0f, 2/32.0f), XMFLOAT2(64, 64) },
	{ XMFLOAT2(2/32.0f, 4/32.0f), XMFLOAT2(2/32.0f, 2/32.0f), XMFLOAT2(64, 64) },
	{ XMFLOAT2(4/32.0f, 4/32.0f), XMFLOAT2(2/32.0f, 2/32.0f), XMFLOAT2(64, 64) },
	{ XMFLOAT2(6/32.0f, 4/32.0f), XMFLOAT2(2/32.0f, 2/32.0f), XMFLOAT2(64, 64) },
	{ XMFLOAT2(6/32.0f, 4/32.0f), XMFLOAT2(2/32.0f, 2/32.0f), XMFLOAT2( 0,  0) },
};
const AnimationData asPlayer[] = {{0, 0.125f}, {1, 0.125f}, {2, 0.125f}};
const AnimationData asPlayerShot[] = {{3, 0.1f}, {4, 0.1f}};
const AnimationData asEnemy[] = {{5, 0.125f}, {6, 0.125f}, {7, 0.125f}, {8, 0.125f}};
const AnimationData asEnemyShot[] = {{9, 0.1f}, {10, 0.1f}};
const AnimationData asBlast[] = {{11, 0.125f}, {12, 0.125f}, {13, 0.125f}, {14, 0.125f}, {15, 0}};
AnimationList animationList;

enum {
	ASID_Player,
	ASID_PlayerShot,
	ASID_Enemy,
	ASID_EnemyShot,
	ASID_Blast,
	ASID_Unused,
};
static const size_t playerShotMax = 32;
static const size_t enemyMax = 64;
static const size_t enemyShotMax = 128;

static const float zPlayer = 0.4f;
static const float zEnemyShot = 0.5f;
static const float zPlayerShot = 0.6f;
static const float zBlast = 0.7f;
static const float zEnemy = 0.8f;

/**
* アニメーションデータを作成する.
*/
void CreateAnimationList()
{
	animationList.list.clear();
	animationList.list.resize(6);
	for (const auto& e : asPlayer) {
		animationList.list[0].push_back({ e.cellIndex, e.time, 0, {1, 1}, {1, 1, 1, 1 } });
	}
	for (const auto& e : asPlayerShot) {
		animationList.list[1].push_back({ e.cellIndex, e.time, 0, {1, 1}, {1, 1, 1, 1 } });
	}
	for (const auto& e : asEnemy) {
		animationList.list[2].push_back({ e.cellIndex, e.time, 0, {1, 1}, {1, 1, 1, 1 } });
	}
	for (const auto& e : asEnemyShot) {
		animationList.list[3].push_back({ e.cellIndex, e.time, 0, {1, 1}, {1, 1, 1, 1 } });
	}
	for (const auto& e : asBlast) {
		animationList.list[4].push_back({ e.cellIndex, e.time, 0, {1, 1}, {1, 1, 1, 1 } });
	}
	animationList.list[5].push_back({ 0, 0, 0, {0, 0}, {1, 1, 1, 1 } });
}

/**
* タイトルシーンオブジェクトを作成する.
*
* @return 作成したタイトルシーンオブジェクトへのポインタ.
*/
::Scene::ScenePtr TitleScene::Create()
{
	return ::Scene::ScenePtr(new TitleScene);
}

/**
* コンストラクタ.
*/
TitleScene::TitleScene() : Scene(L"Title")
{
}

/**
*
*/
bool TitleScene::Load(::Scene::Context&)
{
	Graphics::Graphics& graphics = Graphics::Graphics::Get();

	graphics.texMap.Begin();
	if (!graphics.texMap.LoadFromFile(texBackground, L"Res/UnknownPlanet.png")) {
		return false;
	}
	if (!graphics.texMap.LoadFromFile(texLogo, L"Res/Title.png")) {
		return false;
	}
	if (!graphics.texMap.LoadFromFile(texFont, L"Res/FontPhenomena.png")) {
		return false;
	}
	if (!graphics.texMap.LoadFromFile(texCharacter, L"Res/Objects.png")) {
		return false;
	}
	ID3D12CommandList* ppCommandLists[] = { graphics.texMap.End() };
	graphics.commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	cellFile = Sprite::LoadFromJsonFile(L"Res/Cell/CellFont.json");
	animationFile = LoadAnimationFromJsonFile(L"Res/Anm/AnmTitle.json");
	fontCellList = Sprite::LoadFontFromFile(L"Res/FontPhenomena.fnt");

	graphics.WaitForGpu();
	graphics.texMap.ResetLoader();

	sprBackground.push_back(Sprite::Sprite(&animationFile[0], XMFLOAT3(400, 300, 1.0f)));
	sprBackground[0].SetSeqIndex(0);

	{
		static const char text[] = "RaNDoM MiX";
		static const XMFLOAT2 scale(2, 4);
		const float len = Sprite::GetTextWidth(fontCellList, text) * scale.x;
		XMFLOAT3 textPos(400 - len * 0.5f, 50, 0.8f);
		for (const char c : text) {
			sprLogo.push_back(Sprite::Sprite(nullptr, textPos, 0, scale));
			sprLogo.back().SetCellIndex(c);
			sprLogo.back().color[0] = XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f);
			sprLogo.back().color[1] = XMFLOAT4(0.25f, 0.125f, 1.0f, 1.0f);
			textPos.x += fontCellList.list[c].xadvance * scale.x;
		}
	}
	{
		static const char text[] = "Push any button to start";
		static const XMFLOAT2 scale(1, 1);
		const float len = Sprite::GetTextWidth(fontCellList, text) * scale.x;
		XMFLOAT3 textPos(400 - len * 0.5f, 400, 0.8f);
		for (const char c : text) {
			sprFont.push_back(Sprite::Sprite(nullptr, textPos, 0, scale));
			sprFont.back().SetCellIndex(c);
			sprFont.back().color[1] = XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f);
			sprFont.back().color[0] = XMFLOAT4(0.25f, 0.5f, 1.0f, 1.0f);
			textPos.x += fontCellList.list[c].xadvance * scale.x;
		}
	}

	seStart = Audio::Engine::Get().Prepare(L"Res/SE/Start.wav");
	sePlayerShot = Audio::Engine::Get().Prepare(L"Res/SE/PlayerShot.wav");
	seBlast = Audio::Engine::Get().Prepare(L"Res/SE/Bomb.wav");

	time = 0.0f;
	started = 0.0f;

	CreateAnimationList();
	entryTimer = 5;
	sprPlayer.resize(playerShotMax + 1, Sprite::Sprite(&animationList, {}));
	sprPlayer[0].pos = { 400, 600 - 64, zPlayer };
	sprPlayerShotFree.resize(playerShotMax);
	for (size_t i = 0; i < playerShotMax; ++i) {
		sprPlayer[i + 1].animeController.SetSeqIndex(ASID_Unused);
		sprPlayerShotFree[i] = &sprPlayer[i + 1];
	}
	sprEnemy.resize(enemyMax + enemyShotMax, Sprite::Sprite(&animationList, {}));
	sprEnemyFree.resize(enemyMax);
	sprEnemyShotFree.resize(enemyShotMax);
	for (size_t i = 0; i < enemyMax; ++i) {
		sprEnemy[i].animeController.SetSeqIndex(ASID_Unused);
		sprEnemyFree[i] = &sprEnemy[i];
	}
	for (size_t i = 0; i < enemyShotMax; ++i) {
		sprEnemy[i + enemyMax].animeController.SetSeqIndex(ASID_Unused);
		sprEnemyShotFree[i] = &sprEnemy[i + enemyMax];
	}
	rnd.seed(std::random_device()());

	return true;
}

/**
*
*/
bool TitleScene::Unload(::Scene::Context&)
{
	return true;
}

void TitleScene::UpdatePlayer(double delta)
{
	const GamePad& gamepad = GetGamePad(GamePadId_1P);
	XMFLOAT2 vec = { 0, 0 };
	if (gamepad.buttons & GamePad::DPAD_LEFT) {
		vec.x = -1;
	}
	if (gamepad.buttons & GamePad::DPAD_RIGHT) {
		vec.x = 1;
	}
	if (gamepad.buttons & GamePad::DPAD_UP) {
		vec.y = -1;
	}
	if (gamepad.buttons & GamePad::DPAD_DOWN) {
		vec.y = 1;
	}
	const float speed = static_cast<float>(400 * delta);
	XMStoreFloat2(&vec, XMVector2Normalize(XMLoadFloat2(&vec)) * XMVECTOR{ speed, speed, 0, 0 });
	sprPlayer[0].pos.x = std::min(800.0f - 32.0f, std::max(32.0f, sprPlayer[0].pos.x + vec.x));
	sprPlayer[0].pos.y = std::min(600.0f - 32.0f, std::max(32.0f, sprPlayer[0].pos.y + vec.y));

	if (gamepad.buttonDown & GamePad::A) {
		for (float i = 0; i < 2; ++i) {
			if (sprPlayerShotFree.empty()) {
				break;
			}
			Sprite::Sprite* p = sprPlayerShotFree.back();
			sprPlayerShotFree.pop_back();
			p->pos = { sprPlayer[0].pos.x + 24.0f - 42.0f * i, sprPlayer[0].pos.y - 8.0f, zPlayerShot };
			p->velocity = { 0, -1000 };
			p->animeController.SetSeqIndex(ASID_PlayerShot);
			p->collisionShape = Collision::Shape::MakeRectangle({ -16, -32 }, { 16, 32 });
		}
		sePlayerShot->Play();
	}
	sprPlayer[0].Update(delta);
}

void TitleScene::UpdatePlayerShot(double delta)
{
	for (auto itr = sprPlayer.begin() + 1; itr != sprPlayer.end(); ++itr) {
		if (itr->animeController.GetSeqIndex() == ASID_Unused) {
			continue;
		}
		itr->Update(delta);
		XMStoreFloat3(&itr->pos, XMLoadFloat3(&itr->pos) + XMLoadFloat2(&itr->velocity) * XMVectorReplicate(static_cast<float>(delta)));
		if (itr->pos.x < -32 || itr->pos.x > 832 || itr->pos.y < -32 || itr->pos.y > 632) {
			itr->animeController.SetSeqIndex(ASID_Unused);
			sprPlayerShotFree.push_back(&*itr);
		}
	}
}

void TitleScene::UpdateEnemy(double delta)
{
	const auto begin = sprEnemy.begin();
	const auto end = begin + enemyMax;
	for (auto itr = begin; itr != end; ++itr) {
		const uint32_t seq = itr->animeController.GetSeqIndex();
		if (seq == ASID_Unused) {
			continue;
		}
		itr->Update(delta);
		if (seq == ASID_Blast) {
			if (itr->animeController.IsFinished()) {
				itr->animeController.SetSeqIndex(ASID_Unused);
				sprEnemyFree.push_back(&*itr);
			}
			continue;
		}
		itr->pos.y += static_cast<float>(200 * delta);
		if (itr->pos.y > 200) {
			const float offset = itr->pos.x < 400 ? -4.0f : 4.0f;
			itr->pos.x += offset * (itr->pos.y - 200) * static_cast<float>(delta);

			if (!itr->param[0]) {
				itr->param[0] = true;
				if (sprEnemyShotFree.empty()) {
					break;
				}
				Sprite::Sprite* p = sprEnemyShotFree.back();
				sprEnemyShotFree.pop_back();
				p->pos = { itr->pos.x, itr->pos.y, zEnemyShot };
				const XMVECTOR vec = XMVector2Normalize(XMLoadFloat3(&sprPlayer[0].pos) - XMLoadFloat3(&p->pos));
				XMStoreFloat2(&p->velocity, vec * XMVectorReplicate(300.0f));
				const float angle = std::acos(XMVectorGetY(vec));
				p->rotation = XMVectorGetX(vec) < 0.0f ? angle : -angle;
				p->animeController.SetSeqIndex(ASID_EnemyShot);
			}
		}
		if (itr->pos.x < -64 || itr->pos.x > 864 || itr->pos.y < -64 || itr->pos.y > 664) {
			itr->animeController.SetSeqIndex(ASID_Unused);
			sprEnemyFree.push_back(&*itr);
		}
	}
}

void TitleScene::UpdateEnemyShot(double delta)
{
	const auto begin = sprEnemy.begin() + enemyMax;
	const auto end = begin + enemyShotMax;
	for (auto itr = begin; itr != end; ++itr) {
		if (itr->animeController.GetSeqIndex() == ASID_Unused) {
			continue;
		}
		itr->Update(delta);
		XMStoreFloat3(&itr->pos, XMLoadFloat3(&itr->pos) + XMLoadFloat2(&itr->velocity) * XMVectorReplicate(static_cast<float>(delta)));
		if (itr->pos.x < -32 || itr->pos.x > 832 || itr->pos.y < -32 || itr->pos.y > 632) {
			itr->animeController.SetSeqIndex(ASID_Unused);
			sprEnemyShotFree.push_back(&*itr);
		}
	}
}

void TitleScene::GenerateEnemy(double delta)
{
	entryTimer -= delta;
	if (entryTimer <= 0) {
		entryTimer = std::uniform_int_distribution<>(1, 3)(rnd);
		int popCount = std::uniform_int_distribution<>(2, 4)(rnd);
		const float posX = static_cast<float>(std::uniform_int_distribution<>(64, 800 - 64)(rnd));
		const float offsetX = posX < 400.0f ? 64.0f : -64.0f;
		for (int i = 0; i < popCount; ++i) {
			if (sprEnemyFree.empty()) {
				break;
			}
			Sprite::Sprite* p = sprEnemyFree.back();
			sprEnemyFree.pop_back();
			p->pos = { posX + offsetX * i, static_cast<float>(-16 * i), zEnemy };
			p->animeController.SetSeqIndex(ASID_Enemy);
			p->collisionShape = Collision::Shape::MakeRectangle({ -28, -28 }, { 28, 28 });
			p->param[0] = false;
		}
	}
}

void TitleScene::CollisionPlayerShotAndEnemy()
{
	auto end0 = sprPlayer.begin() + 1 + playerShotMax;
	auto end1 = sprEnemy.begin() + enemyMax;
	for (auto itr0 = sprPlayer.begin() + 1; itr0 != end0; ++itr0) {
		if (itr0->animeController.GetSeqIndex() == ASID_Unused) {
			continue;
		}
		const XMFLOAT2 pos0(itr0->pos.x, itr0->pos.y);
		for (auto itr1 = sprEnemy.begin(); itr1 != end1; ++itr1) {
			const uint32_t seq = itr1->animeController.GetSeqIndex();
			if (seq == ASID_Unused || seq == ASID_Blast) {
				continue;
			}
			const XMFLOAT2 pos1(itr1->pos.x, itr1->pos.y);
			if (!Collision::IsCollision(itr0->collisionShape, pos0, itr1->collisionShape, pos1)) {
				continue;
			}
			itr0->animeController.SetSeqIndex(ASID_Unused);
			sprPlayerShotFree.push_back(&*itr0);
			itr1->animeController.SetSeqIndex(ASID_Blast);
			seBlast->Play();
			break;
		}
	}
}

/**
* 更新関数.
*
* @param delta 前回の呼び出しからの経過時間.
*
* @return 終了コード.
*/
int TitleScene::Update(::Scene::Context&, double delta)
{
	time += delta;

	{
		GenerateEnemy(delta);
		UpdatePlayer(delta);
		UpdatePlayerShot(delta);
		UpdateEnemy(delta);
		UpdateEnemyShot(delta);
		CollisionPlayerShotAndEnemy();
	}

	const float brightness = static_cast<float>(std::fabs(std::fmod(time, 2.0) - 1.0));
	for (auto& e : sprFont) {
		e.color[0].w = brightness;
		e.color[1].w = brightness;
	}

	for (Sprite::Sprite& sprite : sprBackground) {
		sprite.Update(delta);
	}
	for (Sprite::Sprite& sprite : sprLogo) {
		sprite.Update(delta);
	}
	for (Sprite::Sprite& sprite : sprFont) {
		sprite.Update(delta);
	}

	if (started) {
		if (seStart->GetState() & Audio::State_Stopped) {
			return ExitCode_MainGame;
		}
	} else {
		const GamePad gamepad = GetGamePad(GamePadId_1P);
		if (gamepad.buttonDown & (GamePad::START)) {
			seStart->Play();
			started = true;
		}
		static uint32_t vibSeqNo = 0;
		if (gamepad.buttonDown & GamePad::X) {
			VibrateGamePad(GamePadId_1P, vibSeqNo);
		}
		if (gamepad.buttonDown & GamePad::L) {
			++vibSeqNo;
			if (vibSeqNo >= GetVibrationListSize()) {
				vibSeqNo = 0;
			}
		}
		if (gamepad.buttonDown & GamePad::R) {
			if (vibSeqNo <= 0) {
				vibSeqNo = GetVibrationListSize();
			}
			--vibSeqNo;
		}
	}
	return ExitCode_Continue;
}

/**
* シーンの描画.
*/
void TitleScene::Draw(Graphics::Graphics& graphics) const
{
	Sprite::RenderingInfo spriteRenderingInfo;
	spriteRenderingInfo.rtvHandle = graphics.GetRTVHandle();
	spriteRenderingInfo.dsvHandle = graphics.GetDSVHandle();
	spriteRenderingInfo.viewport = graphics.viewport;
	spriteRenderingInfo.scissorRect = graphics.scissorRect;
	spriteRenderingInfo.texDescHeap = graphics.csuDescriptorHeap.Get();
	spriteRenderingInfo.matViewProjection = graphics.matViewProjection;

	const PSO& pso = GetPSO(PSOType_Sprite);
	graphics.spriteRenderer.Draw(sprBackground, bgCellList, pso, texBackground, spriteRenderingInfo);
	graphics.spriteRenderer.Draw(sprLogo, fontCellList.list.data(), pso, texFont, spriteRenderingInfo);
	graphics.spriteRenderer.Draw(sprFont, fontCellList.list.data(), pso, texFont, spriteRenderingInfo);

	graphics.spriteRenderer.Draw(sprEnemy, charCellList, pso, texCharacter, spriteRenderingInfo);
	graphics.spriteRenderer.Draw(sprPlayer, charCellList, pso, texCharacter, spriteRenderingInfo);
}
