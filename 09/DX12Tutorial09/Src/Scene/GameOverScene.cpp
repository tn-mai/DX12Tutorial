/**
* @file GameOverScene.cpp
*/
#include "GameOverScene.h"
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
*
*/
::Scene::ScenePtr GameOverScene::Create()
{
	return ::Scene::ScenePtr(new GameOverScene);
}

/**
* コンストラクタ.
*/
GameOverScene::GameOverScene() : Scene(L"GameOver")
{
}

/**
*
*/
bool GameOverScene::Load()
{
	::Scene::Graphics& graphics = ::Scene::Graphics::Get();

	graphics.texMap.Begin();
	if (!graphics.texMap.LoadFromFile(texBackground, L"Res/UnknownPlanet.png")) {
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

	static const char text[] = "GAME OVER";
	XMFLOAT3 textPos(400 - (_countof(text) - 2) * 16, 300, 0.8f);
	for (const char c : text) {
		if (c >= ' ' && c < '`') {
			sprFont.push_back(Sprite::Sprite(animationFile[1], textPos, 0, XMFLOAT2(1, 1), XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f)));
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
bool GameOverScene::Unload()
{
	return true;
}

/**
*
*/
int GameOverScene::Update(double delta)
{
	time += delta;
	const float brightness = static_cast<float>(std::fabs(std::fmod(time, 2.0) - 1.0));
	for (auto& e : sprFont) {
		e.color.w = brightness;
	}

	for (Sprite::Sprite& sprite : sprBackground) {
		sprite.animeController.Update(delta);
	}
	for (Sprite::Sprite& sprite : sprFont) {
		sprite.animeController.Update(delta);
	}

	if (time > 2) {
		const GamePad gamepad = GetGamePad(GamePadId_1P);
		if (gamepad.trigger & (GamePad::A | GamePad::B | GamePad::START)) {
			return ExitCode_Exit;
		}
	}
	return ExitCode_Continue;
}

/**
*
*/
void GameOverScene::Draw(::Scene::Graphics& graphics) const
{
	Sprite::RenderingInfo spriteRenderingInfo;
	spriteRenderingInfo.rtvHandle = graphics.GetRTVHandle();
	spriteRenderingInfo.dsvHandle = graphics.GetDSVHandle();
	spriteRenderingInfo.viewport = graphics.viewport;
	spriteRenderingInfo.scissorRect = graphics.scissorRect;
	spriteRenderingInfo.texDescHeap = graphics.csuDescriptorHeap.Get();
	spriteRenderingInfo.matViewProjection = graphics.matViewProjection;

	graphics.spriteRenderer.Draw(sprBackground, cellList, GetPSO(PSOType_Sprite), texBackground, spriteRenderingInfo);
	graphics.spriteRenderer.Draw(sprFont, cellFile->Get(0)->list.data(), GetPSO(PSOType_Sprite), texFont, spriteRenderingInfo);
}
