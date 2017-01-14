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

	cellFile[0] = Sprite::LoadFromJsonFile(L"Res/Cell/CellFont.json");
	cellFile[1] = Sprite::LoadFromJsonFile(L"Res/Cell/CellEnemy.json");

	animationFile[0] = LoadAnimationFromJsonFile(L"Res/Anm/AnmTitle.json");
	animationFile[1] = LoadAnimationFromJsonFile(L"Res/Anm/Animation.json");

	actionFile = Action::LoadFromJsonFile(L"Res/Act/ActEnemy.json");

	graphics.WaitForGpu();

	sprBackground.push_back(Sprite::Sprite(animationFile[0][0], XMFLOAT3(400, 300, 1.0f)));
	sprBackground[0].SetSeqIndex(0);

	sprObjects.push_back(Sprite::Sprite(animationFile[1][0], XMFLOAT3(400, -16, 0.9f)));
	sprObjects[0].SetSeqIndex(0);
	sprObjects[0].SetActionList(actionFile->Get(0));

	static const char text[] = "00000000";
	XMFLOAT3 textPos(400 - 16 * (sizeof(text) - 1), 32, 0.8f);
	for (char c : text) {
		if (c >= ' ' && c < '`') {
			sprFont.push_back(Sprite::Sprite(animationFile[0][1], textPos, 0, XMFLOAT2(1, 1), XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f)));
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
	const float brightness = static_cast<float>(std::fabs(std::fmod(time, 2.0) - 1.0));
	for (auto& e : sprFont) {
		e.color.w = brightness;
	}

	for (Sprite::Sprite& sprite : sprBackground) {
		sprite.Update(delta);
	}
	for (Sprite::Sprite& sprite : sprObjects) {
		sprite.Update(delta);
	}
	for (Sprite::Sprite& sprite : sprFont) {
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

	graphics.spriteRenderer.Draw(sprBackground, cellList, GetPSO(PSOType_Sprite), texBackground, spriteRenderingInfo);
	graphics.spriteRenderer.Draw(sprObjects, cellFile[1]->Get(0)->list.data(), GetPSO(PSOType_Sprite), texObjects, spriteRenderingInfo);
	graphics.spriteRenderer.Draw(sprFont, cellFile[0]->Get(0)->list.data(), GetPSO(PSOType_Sprite), texFont, spriteRenderingInfo);
}
