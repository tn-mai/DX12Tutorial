/**
* @file TitleScene.cpp
*/
#include "TitleScene.h"
#include "../Graphics.h"
#include "../PSO.h"
#include "../GamePad.h"
#include <DirectXMath.h>

using namespace DirectX;

/**
* スプライト用セルデータ.
*/
const Sprite::Cell cellList[] = {
	{ XMFLOAT2(0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT2(800, 600) },

	{ XMFLOAT2(16.0f / 1024.0f, 48.0f / 512.0f), XMFLOAT2(480.0f / 1024.0f, 256.0f / 512.0f), XMFLOAT2(480, 256) },
};

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
	if (!graphics.texMap.LoadFromFile(texFont, L"Res/TextFont.png")) {
		return false;
	}
	ID3D12CommandList* ppCommandLists[] = { graphics.texMap.End() };
	graphics.commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	cellFile = Sprite::LoadFromJsonFile(L"Res/Cell/CellFont.json");
	animationFile = LoadAnimationFromJsonFile(L"Res/Anm/AnmTitle.json");

	graphics.WaitForGpu();
	graphics.texMap.ResetLoader();

	sprBackground.push_back(Sprite::Sprite(animationFile[0], XMFLOAT3(400, 300, 1.0f)));
	sprBackground[0].SetSeqIndex(0);

	sprLogo.push_back(Sprite::Sprite(animationFile[0], XMFLOAT3(400, 200, 0.9f)));
	sprLogo[0].SetSeqIndex(1);

	static const char text[] = "START";
	XMFLOAT3 textPos(400 - (_countof(text) - 2) * 16, 400, 0.8f);
	for (const char c : text) {
		if (c >= ' ' && c < '`') {
			sprFont.push_back(Sprite::Sprite(animationFile[1], textPos, 0, XMFLOAT2(1, 1), XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f)));
			sprFont.back().SetSeqIndex(c - ' ');
			textPos.x += 32.0f;
		}
	}

	seStart = Audio::Engine::Get().Prepare(L"Res/SE/Start.wav");

	time = 0.0f;
	started = 0.0f;

	return true;
}

/**
*
*/
bool TitleScene::Unload(::Scene::Context&)
{
	return true;
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
	const float brightness = static_cast<float>(std::fabs(std::fmod(time, 2.0) - 1.0));
	for (auto& e : sprFont) {
		e.color.w = brightness;
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
		if (gamepad.buttonDown & (GamePad::A | GamePad::B | GamePad::START)) {
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

	graphics.spriteRenderer.Draw(sprBackground, cellList, GetPSO(PSOType_Sprite), texBackground, spriteRenderingInfo);
	graphics.spriteRenderer.Draw(sprLogo, cellList, GetPSO(PSOType_Sprite), texLogo, spriteRenderingInfo);
	graphics.spriteRenderer.Draw(sprFont, cellFile->Get(0)->list.data(), GetPSO(PSOType_Sprite), texFont, spriteRenderingInfo);
}
