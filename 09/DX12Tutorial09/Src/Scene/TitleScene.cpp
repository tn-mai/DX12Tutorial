/**
* @file TitleScene.cpp
*/
#include "TitleScene.h"
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
*
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
bool TitleScene::Load()
{
	::Scene::Graphics& graphics = ::Scene::Graphics::Get();
	Resource::ResourceLoader loader;
	if (!loader.Begin(graphics.csuDescriptorHeap)) {
		return false;
	}
	if (!loader.LoadFromFile(texBackground, 0, L"Res/UnknownPlanet.png")) {
		return false;
	}
	if (!loader.LoadFromFile(texLogo, 1, L"Res/Title.png")) {
		return false;
	}
	if (!loader.LoadFromFile(texFont, 2, L"Res/TextFont.png")) {
		return false;
	}
	ID3D12CommandList* ppCommandLists[] = { loader.End() };
	graphics.commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	animationFile = LoadAnimationFromJsonFile(L"Res/Anm/AnmTitle.json");

	graphics.WaitForGpu();

	sprBackground.push_back(Sprite::Sprite(animationFile[0], XMFLOAT3(400, 300, 1.0f)));
	sprBackground[0].SetSeqIndex(0);

	sprLogo.push_back(Sprite::Sprite(animationFile[0], XMFLOAT3(400, 200, 0.9f)));
	sprLogo[0].SetSeqIndex(1);

	static const char text[] = "START";
	XMFLOAT3 textPos(320, 400, 0.8f);
	for (char c : text) {
		if (c >= ' ' && c < '`') {
			sprFont.push_back(Sprite::Sprite(animationFile[0], textPos, 0, XMFLOAT2(1, 1), XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f)));
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
bool TitleScene::Unload()
{
	return true;
}

/**
*
*/
int TitleScene::Update(double delta)
{
	time += delta;
	const float brightness = static_cast<float>(std::fabs(std::fmod(time, 2.0) - 1.0));
	for (auto& e : sprFont) {
		e.color.w = brightness;
	}
	return ExitCode_Continue;
}

/**
*
*/
void TitleScene::Draw(::Scene::Graphics& graphics) const
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
	graphics.spriteRenderer.Draw(sprFont, cellList, GetPSO(PSOType_Sprite), texFont, spriteRenderingInfo);
}
