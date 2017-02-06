/**
* @file EndingScene.cpp
*/
#include "EndingScene.h"
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
*
*/
::Scene::ScenePtr EndingScene::Create()
{
	return ::Scene::ScenePtr(new EndingScene);
}

/**
* コンストラクタ.
*/
EndingScene::EndingScene() : Scene(L"Ending")
{
}

/**
*
*/
bool EndingScene::Load(::Scene::Context& context)
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

	sprBackground.push_back(Sprite::Sprite(animationFile[0], XMFLOAT3(400, 300, 1.0f), 0, XMFLOAT2(1, 1), XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f)));
	sprBackground[0].SetSeqIndex(0);

	sprLogo.push_back(Sprite::Sprite(animationFile[0], XMFLOAT3(400, 200, 0.9f), 0, XMFLOAT2(1, 1), XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f)));
	sprLogo[0].SetSeqIndex(1);
	{
		static const char text[] = "CONGRATULATION";
		XMFLOAT3 textPos(400 - (_countof(text) - 2) * 24.0f, 348, 0.8f);
		for (const char c : text) {
			if (c >= ' ' && c < '`') {
				sprFont.push_back(Sprite::Sprite(animationFile[1], textPos, 0, XMFLOAT2(1.5f, 1.5f), XMFLOAT4(0.5f, 0.0f, 1.0f, 1.0f)));
				sprFont.back().SetSeqIndex(c - ' ');
				textPos.x += 48.0f;
			}
		}
	}
	{
		char text[32];
		const int len = snprintf(text, _countof(text), "SCORE/%08d", context.score);
		XMFLOAT3 textPos(400 - static_cast<float>(len - 1) * 16.0f, 500, 0.8f);
		bool isNumber = false;
		float alpha = 1.0f;
		for (int i = 0; i < len; ++i) {
			const char c = text[i];
			if (c >= ' ' && c < '`') {
				if (c > '0') {
					alpha = 1.0f;
				}
				sprFont.push_back(Sprite::Sprite(animationFile[1], textPos, 0, XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.5f, 1.0f, 0.5f, alpha)));
				sprFont.back().SetSeqIndex(c - ' ');
				textPos.x += 32.0f;
				if (!isNumber && c == '/') {
					isNumber = true;
					alpha = 0.5f;
				}
			}
		}
	}
	time = 0.0f;

	return true;
}

/**
*
*/
bool EndingScene::Unload(::Scene::Context&)
{
	return true;
}

/**
*
*/
int EndingScene::Update(::Scene::Context&, double delta)
{
	time += delta;
	const float brightness = static_cast<float>(std::fabs(std::fmod(time, 2.0) - 1.0));
	for (auto& e : sprFont) {
		if (e.color.y == 0.0f) {
			e.color.w = brightness;
		}
	}

	for (Sprite::Sprite& sprite : sprBackground) {
		sprite.animeController.Update(delta);
	}
	for (Sprite::Sprite& sprite : sprLogo) {
		sprite.animeController.Update(delta);
	}
	for (Sprite::Sprite& sprite : sprFont) {
		sprite.animeController.Update(delta);
	}

	const GamePad gamepad = GetGamePad(GamePadId_1P);
	if (gamepad.buttonDown & GamePad::START) {
		return ExitCode_Exit;
	}
	return ExitCode_Continue;
}

/**
*
*/
void EndingScene::Draw(Graphics::Graphics& graphics) const
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
