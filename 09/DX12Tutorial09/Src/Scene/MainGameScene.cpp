/**
* @file MainGameScene.cpp
*/
#include "MainGameScene.h"
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

	{ XMFLOAT2( 0.0f / 16.0f, 0.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 1.0f / 16.0f, 0.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 2.0f / 16.0f, 0.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 3.0f / 16.0f, 0.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 4.0f / 16.0f, 0.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 5.0f / 16.0f, 0.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 6.0f / 16.0f, 0.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 7.0f / 16.0f, 0.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 8.0f / 16.0f, 0.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 9.0f / 16.0f, 0.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(10.0f / 16.0f, 0.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(11.0f / 16.0f, 0.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(12.0f / 16.0f, 0.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(13.0f / 16.0f, 0.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(14.0f / 16.0f, 0.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(15.0f / 16.0f, 0.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },

	{ XMFLOAT2( 0.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 2.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 3.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 4.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 5.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 6.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 7.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 8.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 9.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(10.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(11.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(12.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(13.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(14.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(15.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },

	{ XMFLOAT2( 0.0f / 16.0f, 4.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 1.0f / 16.0f, 4.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 2.0f / 16.0f, 4.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 3.0f / 16.0f, 4.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 4.0f / 16.0f, 4.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 5.0f / 16.0f, 4.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 6.0f / 16.0f, 4.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 7.0f / 16.0f, 4.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 8.0f / 16.0f, 4.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 9.0f / 16.0f, 4.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(10.0f / 16.0f, 4.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(11.0f / 16.0f, 4.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(12.0f / 16.0f, 4.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(13.0f / 16.0f, 4.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(14.0f / 16.0f, 4.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(15.0f / 16.0f, 4.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },

	{ XMFLOAT2( 1.0f / 16.0f, 6.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 0.0f / 16.0f, 6.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 2.0f / 16.0f, 6.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 3.0f / 16.0f, 6.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 4.0f / 16.0f, 6.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 5.0f / 16.0f, 6.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 6.0f / 16.0f, 6.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 7.0f / 16.0f, 6.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 8.0f / 16.0f, 6.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2( 9.0f / 16.0f, 6.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(10.0f / 16.0f, 6.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(11.0f / 16.0f, 6.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(12.0f / 16.0f, 6.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(13.0f / 16.0f, 6.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(14.0f / 16.0f, 6.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(15.0f / 16.0f, 6.0f / 16.0f), XMFLOAT2(1.0f / 16.0f, 2.0f / 16.0f), XMFLOAT2(32, 64) },
};

/**
* スプライト用セルデータ.
*/
const Sprite::Cell cellListObjects[] = {
	{ XMFLOAT2(0.0f / 32.0f, 0.0f / 32.0f), XMFLOAT2(2.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(64, 64) },
	{ XMFLOAT2(2.0f / 32.0f, 0.0f / 32.0f), XMFLOAT2(2.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(64, 64) },
	{ XMFLOAT2(4.0f / 32.0f, 0.0f / 32.0f), XMFLOAT2(2.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(64, 64) },
	{ XMFLOAT2(6.0f / 32.0f, 0.0f / 32.0f), XMFLOAT2(2.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(64, 64) },

	{ XMFLOAT2(0.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(1.0f / 32.0f, 1.0f / 32.0f), XMFLOAT2(32, 32) },
	{ XMFLOAT2(1.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(1.0f / 32.0f, 1.0f / 32.0f), XMFLOAT2(32, 32) },
	{ XMFLOAT2(2.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(1.0f / 32.0f, 1.0f / 32.0f), XMFLOAT2(32, 32) },

	{ XMFLOAT2(3.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(1.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(4.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(1.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(32, 64) },
	{ XMFLOAT2(5.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(1.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(32, 64) },

	{ XMFLOAT2(0.0f / 32.0f, 3.0f / 32.0f), XMFLOAT2(1.0f / 32.0f, 1.0f / 32.0f), XMFLOAT2(32, 32) },

	{ XMFLOAT2(0.0f / 32.0f, 4.0f / 32.0f), XMFLOAT2(2.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(64, 64) },
	{ XMFLOAT2(2.0f / 32.0f, 4.0f / 32.0f), XMFLOAT2(2.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(64, 64) },
	{ XMFLOAT2(4.0f / 32.0f, 4.0f / 32.0f), XMFLOAT2(2.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(64, 64) },
	{ XMFLOAT2(6.0f / 32.0f, 4.0f / 32.0f), XMFLOAT2(2.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(64, 64) },
	{ XMFLOAT2(6.0f / 32.0f, 4.0f / 32.0f), XMFLOAT2(2.0f / 32.0f, 2.0f / 32.0f), XMFLOAT2(0, 0) },
};

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

	animationFile[0] = LoadAnimationFromJsonFile(L"Res/Anm/AnmTitle.json");
	animationFile[1] = LoadAnimationFromJsonFile(L"Res/Anm/Animation.json");

	graphics.WaitForGpu();

	sprBackground.push_back(Sprite::Sprite(animationFile[0][0], XMFLOAT3(400, 300, 1.0f)));
	sprBackground[0].SetSeqIndex(0);

	sprObjects.push_back(Sprite::Sprite(animationFile[1][0], XMFLOAT3(400, 200, 0.9f)));
	sprObjects[0].SetSeqIndex(0);

	static const char text[] = "00000000";
	XMFLOAT3 textPos(400 - 16 * (sizeof(text) - 1), 32, 0.8f);
	for (char c : text) {
		if (c >= ' ' && c < '`') {
			sprFont.push_back(Sprite::Sprite(animationFile[0][0], textPos, 0, XMFLOAT2(1, 1), XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f)));
			sprFont.back().SetSeqIndex(c - ' ' + 2);
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
	const float brightness = static_cast<float>(std::fabs(std::fmod(time, 2.0) - 1.0));
	for (auto& e : sprFont) {
		e.color.w = brightness;
	}

	for (Sprite::Sprite& sprite : sprBackground) {
		sprite.animeController.Update(delta);
	}
	for (Sprite::Sprite& sprite : sprObjects) {
		sprite.animeController.Update(delta);
	}
	for (Sprite::Sprite& sprite : sprFont) {
		sprite.animeController.Update(delta);
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

	graphics.spriteRenderer.Draw(sprBackground, cellList, GetPSO(PSOType_Sprite), texBackground, spriteRenderingInfo);
	graphics.spriteRenderer.Draw(sprObjects, cellListObjects, GetPSO(PSOType_Sprite), texObjects, spriteRenderingInfo);
	graphics.spriteRenderer.Draw(sprFont, cellList, GetPSO(PSOType_Sprite), texFont, spriteRenderingInfo);
}
