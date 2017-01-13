/**
* @file Main.cpp
*/
#include <Windows.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "Scene.h"
#include "Texture.h"
#include "PSO.h"
#include "Sprite.h"
#include "Timer.h"
#include "GamePad.h"

#include "Scene/TitleScene.h"
#include "Scene/MainGameScene.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

const wchar_t windowClassName[] = L"DX12TutorialApp";
const wchar_t windowTitle[] = L"DX12Tutorial";
const int clientWidth = 800;
const int clientHeight = 600;
HWND hwnd = nullptr;

HRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

enum Id
{
	Id_None = -1,
	Id_Title,
	Id_MainGame,
	Id_Ending,
	Id_Option,
	Id_Pause,
	Id_GameOver,
};

enum PauseExitCode
{
	PauseExitCode_Option,
};

Scene::ScenePtr MakeScene() { return Scene::ScenePtr(); }

static const Scene::Creator creatorList[] = {
	{ Id_Title, TitleScene::Create },
	{ Id_MainGame, MainGameScene::Create },
	{ Id_Ending, MakeScene },
	{ Id_Option, MakeScene },
	{ Id_Pause, MakeScene },
	{ Id_GameOver, MakeScene },
};

const Scene::Transition transitionList[] = {
	{ Id_Title, { TitleScene::ExitCode_MainGame, Scene::TransitionType::Jump, Id_MainGame } },
	{ Id_Title, { TitleScene::ExitCode_Option, Scene::TransitionType::Push, Id_Option } },
	{ Id_MainGame, { MainGameScene::ExitCode_Ending, Scene::TransitionType::Jump, Id_Ending } },
	{ Id_MainGame, { MainGameScene::ExitCode_Pause, Scene::TransitionType::Push, Id_Pause } },
	{ Id_MainGame, { MainGameScene::ExitCode_GameOver, Scene::TransitionType::Jump, Id_GameOver } },
	{ Id_Ending, { Scene::Scene::ExitCode_Exit, Scene::TransitionType::Jump, Id_Title } },
	{ Id_Option, { Scene::Scene::ExitCode_Exit, Scene::TransitionType::Pop, Id_None } },
	{ Id_Pause,{ Scene::Scene::ExitCode_Exit, Scene::TransitionType::Pop, Id_None } },
	{ Id_Pause, { PauseExitCode_Option, Scene::TransitionType::Push, Id_Option } },
	{ Id_GameOver, { Scene::Scene::ExitCode_Exit, Scene::TransitionType::Jump, Id_Title } },
};

Resource::Texture texNoise;
Resource::Texture texBackground;

std::vector<Sprite::Sprite> spriteList;
Resource::Texture texSprite;

ComPtr<ID3D12Resource> vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

ComPtr<ID3D12Resource> indexBuffer;
D3D12_INDEX_BUFFER_VIEW indexBufferView;

Scene::Context sceneContext;

bool InitializeD3D();
void FinalizeD3D();
bool Render();
void Update(double delta);

bool CreateVertexBuffer();
bool CreateIndexBuffer();
bool LoadTexture();
bool CreateNoiseTexture(Resource::ResourceLoader&);
void DrawTriangle();
void DrawRectangle();

/// 頂点データ型.
struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT4 color;
	XMFLOAT2 texcoord;
};

/**
* 頂点データ配列.
*/
static const Vertex vertices[] = {
	{ XMFLOAT3(   0, 150, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.5f, 0.0f) },
	{ XMFLOAT3( 200,-150, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-200,-150, 0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(-120, 120, 0.4f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(  80, 120, 0.4f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(  80, -30, 0.4f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-120, -30, 0.4f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3( -80,  30, 0.6f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3( 120,  30, 0.6f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3( 120,-120, 0.6f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3( -80,-120, 0.6f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(-400, 300, 0.9f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3( 400, 300, 0.9f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3( 400,-300, 0.9f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-400,-300, 0.9f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
};

/**
* インデックスデータ配列.
*/
static const uint32_t indices[] = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4,
	8, 9,10,10,11, 8,
};

/// 三角形の描画で使用する頂点数.
const UINT triangleVertexCount = 3;

/**
* スプライト用セルデータ.
*/
const Sprite::Cell cellList[] = {
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
* エントリポイント.
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wc.lpszClassName = windowClassName;
	RegisterClassEx(&wc);

	RECT windowRect = { 0, 0, clientWidth, clientHeight };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	hwnd = CreateWindowEx(
		0,
		windowClassName,
		windowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	if (!InitializeD3D()) {
		return 0;
	}

	Timer timer;
	for (;;) {
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			break;
		}
		Update(timer.GetFrameDelta());
		if (!Render()) {
			break;
		}
	}
	FinalizeD3D();

	CoUninitialize();

	return 0;
}

HRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	GamePad& gamepad = GetGamePad(GamePadId_1P);
	switch (msg) {
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE) {
			DestroyWindow(hwnd);
			return 0;
		}
		switch (wparam) {
		case 'W': gamepad.buttons |= GamePad::DPAD_UP; break;
		case 'A': gamepad.buttons |= GamePad::DPAD_LEFT; break;
		case 'S': gamepad.buttons |= GamePad::DPAD_DOWN; break;
		case 'D': gamepad.buttons |= GamePad::DPAD_RIGHT; break;
		case VK_SPACE: gamepad.buttons |= GamePad::A; break;
		}
		break;
	case WM_KEYUP:
		switch (wparam) {
		case 'W': gamepad.buttons &= ~GamePad::DPAD_UP; break;
		case 'A': gamepad.buttons &= ~GamePad::DPAD_LEFT; break;
		case 'S': gamepad.buttons &= ~GamePad::DPAD_DOWN; break;
		case 'D': gamepad.buttons &= ~GamePad::DPAD_RIGHT; break;
		case VK_SPACE: gamepad.buttons &= ~GamePad::A; break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

bool InitializeD3D()
{
	Scene::Graphics::Get().Initialize(hwnd, clientWidth, clientHeight);
	if (!CreateVertexBuffer()) {
		return false;
	}
	if (!CreateIndexBuffer()) {
		return false;
	}
	if (!LoadTexture()) {
		return false;
	}

	spriteList.push_back(Sprite::Sprite(GetAnimationList(), XMFLOAT3(100, 100, 0.1f), 0, XMFLOAT2(1, 1), XMFLOAT4(1, 1, 1, 1)));

	sceneContext.Initialize(transitionList, _countof(transitionList), creatorList, _countof(creatorList));
	sceneContext.Start(Id_Title);

	return true;
}

void FinalizeD3D()
{
	Scene::Graphics::Get().Finalize();
}

bool Render()
{
	Scene::Graphics& graphics = Scene::Graphics::Get();
	graphics.BeginRendering();
	graphics.spriteRenderer.Begin(graphics.currentFrameIndex);

	sceneContext.Draw(graphics);

#if 0
	DrawTriangle();
	DrawRectangle();

	Sprite::RenderingInfo spriteRenderingInfo;
	spriteRenderingInfo.rtvHandle = graphics.GetRTVHandle();
	spriteRenderingInfo.dsvHandle = graphics.GetDSVHandle();
	spriteRenderingInfo.viewport = graphics.viewport;
	spriteRenderingInfo.scissorRect = graphics.scissorRect;
	spriteRenderingInfo.texDescHeap = graphics.csuDescriptorHeap.Get();
	spriteRenderingInfo.matViewProjection = graphics.matViewProjection;
	graphics.spriteRenderer.Draw(spriteList, cellList, GetPSO(PSOType_Sprite), texSprite, spriteRenderingInfo);
#endif

	graphics.spriteRenderer.End();
	graphics.EndRendering();

	return true;
}

/**
* アプリケーションの状態を更新する.
*/
void Update(double delta)
{
	GamePad& gamepad = GetGamePad(GamePadId_1P);
	const float speed = static_cast<float>(200.0 * delta);
	if (gamepad.buttons & GamePad::DPAD_LEFT) {
		spriteList[0].pos.x -= speed;
	} else if (gamepad.buttons & GamePad::DPAD_RIGHT) {
		spriteList[0].pos.x += speed;
	}
	if (gamepad.buttons & GamePad::DPAD_UP) {
		spriteList[0].pos.y -= speed;
	} else if (gamepad.buttons & GamePad::DPAD_DOWN) {
		spriteList[0].pos.y += speed;
	}

	sceneContext.Update(delta);

#if 0
	spriteList[0].rotation += 0.1f;
	if (spriteList[0].rotation >= 3.1415f * 2.0f) {
		spriteList[0].rotation -= 3.1415f * 2.0f;
	}
#endif
	static uint32_t seqNo = 0;
	if (gamepad.buttons & GamePad::A) {
		++seqNo;
		if (seqNo >= spriteList[0].animeController.GetSeqCount()) {
			seqNo = 0;
		}
		spriteList[0].animeController.SetSeqIndex(seqNo);
	}

	for (Sprite::Sprite& sprite : spriteList) {
		sprite.animeController.Update(delta);
	}
}

/**
* 頂点バッファを作成する.
*/
bool CreateVertexBuffer()
{
	Scene::Graphics& graphics = Scene::Graphics::Get();
	if (FAILED(graphics.device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer)))) {
		return false;
	}
	vertexBuffer->SetName(L"Vertex buffer");

	void* pVertexDataBegin;
	const D3D12_RANGE readRange = { 0, 0 };
	if (FAILED(vertexBuffer->Map(0, &readRange, &pVertexDataBegin))) {
		return false;
	}
	memcpy(pVertexDataBegin, vertices, sizeof(vertices));
	vertexBuffer->Unmap(0, nullptr);

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = sizeof(vertices);

	return true;
}

/**
* インデックスバッファを作成する.
*/
bool CreateIndexBuffer()
{
	Scene::Graphics& graphics = Scene::Graphics::Get();
	if (FAILED(graphics.device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuffer)))) {
		return false;
	}
	indexBuffer->SetName(L"Index buffer");

	void* pIndexDataBegin;
	const D3D12_RANGE readRange = { 0, 0 };
	if (FAILED(indexBuffer->Map(0, &readRange, &pIndexDataBegin))) {
		return false;
	}
	memcpy(pIndexDataBegin, indices, sizeof(indices));
	indexBuffer->Unmap(0, nullptr);

	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = sizeof(indices);

	return true;
}

/**
* 三角形を描画する.
*/
void DrawTriangle()
{
	Scene::Graphics& graphics = Scene::Graphics::Get();
	const PSO& pso = GetPSO(PSOType_Simple);
	graphics.commandList->SetPipelineState(pso.pso.Get());
	graphics.commandList->SetGraphicsRootSignature(pso.rootSignature.Get());
	graphics.commandList->SetGraphicsRootDescriptorTable(0, texNoise.handle);
	graphics.commandList->SetGraphicsRoot32BitConstants(1, 16, &graphics.matViewProjection, 0);
	graphics.commandList->RSSetViewports(1, &graphics.viewport);
	graphics.commandList->RSSetScissorRects(1, &graphics.scissorRect);
	graphics.commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	graphics.commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	graphics.commandList->DrawInstanced(triangleVertexCount, 1, 0, 0);
}

/**
* 四角形を描画する.
*/
void DrawRectangle()
{
	Scene::Graphics& graphics = Scene::Graphics::Get();
	const PSO& pso = GetPSO(PSOType_NoiseTexture);
	graphics.commandList->SetPipelineState(pso.pso.Get());
	graphics.commandList->SetGraphicsRootSignature(pso.rootSignature.Get());
	graphics.commandList->SetGraphicsRootDescriptorTable(0, texBackground.handle);
	graphics.commandList->SetGraphicsRoot32BitConstants(1, 16, &graphics.matViewProjection, 0);

	static float scrollOffset = 0.0f;
	graphics.commandList->SetGraphicsRoot32BitConstants(2, 1, &scrollOffset, 0);
	scrollOffset -= 0.002f;

	graphics.commandList->RSSetViewports(1, &graphics.viewport);
	graphics.commandList->RSSetScissorRects(1, &graphics.scissorRect);
	graphics.commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	graphics.commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	graphics.commandList->IASetIndexBuffer(&indexBufferView);
	graphics.commandList->DrawIndexedInstanced(_countof(indices), 1, 0, triangleVertexCount, 0);
}

float NoiseSeed(float x, float y)
{
	float i;
	return std::modf(std::sin(x * 12.9898f + y * 78.233f) * 43758.5453123f, &i);
}

float Noise(float x, float y)
{
	float iy;
	const float fy = std::modf(y, &iy);
	const float uy = fy * fy * (3.0f - 2.0f * fy);
	float ix;
	const float fx = std::modf(x, &ix);
	const float ux = fx * fx * (3.0f - 2.0f * fx);
	const float a = NoiseSeed(ix, iy);
	const float b = NoiseSeed(ix + 1.0f, iy);
	const float c = NoiseSeed(ix, iy + 1.0f);
	const float d = NoiseSeed(ix + 1.0f, iy + 1.0f);
	const float value = (a * (1.0f - ux) + b * ux) + (c - a) * uy * (1.0f - ux) + (d - b) * uy * ux;
	if (value < 0.0f) {
		return 0.0f;
	}
	return value;
}

/**
* テクスチャを作成する.
*/
bool CreateNoiseTexture(Resource::ResourceLoader& loader)
{
	const D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 256, 256, 1, 1);
	std::vector<uint8_t> noise;
	noise.resize(static_cast<size_t>(desc.Width * desc.Height) * 4);
	uint8_t* p = noise.data();
#if 1
	for (float y = 0; y < desc.Height; ++y) {
		const float fy = (y / (desc.Height - 1) * 2.0f) - 1.35f;
		for (float x = 0; x < desc.Width; ++x) {
			const float fx = (x / (desc.Width - 1) * 2.0f) - 1.0f;
			const float distance = std::sqrt(fx * fx + fy * fy);
			const float t = 0.02f / std::abs(0.1f - std::fmod(distance, 0.2f));
			const uint8_t col = t < 1.0f ? static_cast<uint8_t>(t * 255.0f) : 255;
			p[0] = col;
			p[1] = col;
			p[2] = col;
			p[3] = 255;
			p += 4;
		}
	}
#else
	for (float y = 0; y < desc.Height; ++y) {
		const float fy = y / (desc.Height - 1);
		for (float x = 0; x < desc.Width; ++x) {
			const float fx = x / (desc.Width - 1);
			float val = 0.0f;
			float scale = 0.5f;
			float freq = 4.0f;
			for (int i = 0; i < 4; ++i) {
				val += Noise(fx * freq, fy * freq) * scale;
				scale *= 0.5f;
				freq *= 2.0f;
			}
			const uint8_t col = static_cast<uint8_t>(val * 255.0f);
			p[0] = col;
			p[1] = col;
			p[2] = col;
			p[3] = 255;
			p += 4;
		}
	}
#endif
	return loader.Create(texNoise, 1, desc, noise.data(), L"texNoise");
}

/**
* テクスチャを読み込む.
*/
bool LoadTexture()
{
	Scene::Graphics& graphics = Scene::Graphics::Get();
	Resource::ResourceLoader loader;
	if (!loader.Begin(graphics.csuDescriptorHeap)) {
		return false;
	}
	if (!loader.LoadFromFile(texBackground, 0, L"Res/UnknownPlanet.png")) {
		return false;
	}
	if (!CreateNoiseTexture(loader)) {
		return false;
	}
	if (!loader.LoadFromFile(texSprite, 2, L"Res/Objects.png")) {
		return false;
	}
	ID3D12CommandList* ppCommandLists[] = { loader.End() };
	graphics.commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	graphics.WaitForGpu();
	return true;
}