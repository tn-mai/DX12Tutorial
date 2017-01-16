/**
* @file Graphics.cpp
*/
#include "Graphics.h"
#include "PSO.h"
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

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
	{ XMFLOAT3(0, 150, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.5f, 0.0f) },
	{ XMFLOAT3(200,-150, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-200,-150, 0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(-120, 120, 0.4f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(80, 120, 0.4f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(80, -30, 0.4f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-120, -30, 0.4f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(-80,  30, 0.6f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(120,  30, 0.6f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(120,-120, 0.6f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-80,-120, 0.6f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(-400, 300, 0.9f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(400, 300, 0.9f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(400,-300, 0.9f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
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

namespace Graphics {

/**
* コマンドリストを作成.
*/
bool CreateCommandList(ComPtr<ID3D12Device>& device, ComPtr<ID3D12CommandAllocator>& allocator, ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)))) {
		return false;
	}
	if (FAILED(commandList->Close())) {
		return false;
	}
	return true;
}

/**
* 描画環境の初期化.
*
* @param hwnd         ウィンドウハンドル.
* @param clientWidth  ウィンドウの幅.
* @param clientHeight ウィンドウの高さ.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool Graphics::Initialize(HWND hwnd, int clientWidth, int clientHeight)
{
#ifndef NDEBUG
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
			debugController->EnableDebugLayer();
		}
	}
#endif

	ComPtr<IDXGIFactory4> dxgiFactory;
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)))) {
		return false;
	}

	// 機能レベル11を満たすハードウェアアダプタを検索し、そのデバイスインターフェイスを取得する.
	ComPtr<IDXGIAdapter1> dxgiAdapter; // デバイス情報を取得するためのインターフェイス
	int adapterIndex = 0; // 列挙するデバイスのインデックス
	bool adapterFound = false; // 目的のデバイスが見つかったかどうか
	while (dxgiFactory->EnumAdapters1(adapterIndex, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC1 desc;
		dxgiAdapter->GetDesc1(&desc);
		if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
			if (SUCCEEDED(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))) {
				adapterFound = true;
				break;
			}
		}
		++adapterIndex;
	}
	if (!adapterFound) {
		// 機能レベル11を満たすハードウェアが見つからない場合、WARPデバイスの作成を試みる.
		if (FAILED(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter)))) {
			return false;
		}
		warp = true;
	}
	if (FAILED(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) {
		return false;
	}

	// コマンドキューを作成.
	const D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	if (FAILED(device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue)))) {
		return false;
	}

	// スワップチェーンを作成.
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = clientWidth;
	scDesc.Height = clientHeight;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.SampleDesc.Count = 1;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = frameBufferCount;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	ComPtr<IDXGISwapChain1> tmpSwapChain;
	if (FAILED(dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &scDesc, nullptr, nullptr, &tmpSwapChain))) {
		return false;
	}
	tmpSwapChain.As(&swapChain);
	currentFrameIndex = swapChain->GetCurrentBackBufferIndex();

	// RTV用のデスクリプタヒープ及びデスクリプタを作成.
	D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
	rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDesc.NumDescriptors = frameBufferCount;
	rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&rtvDescriptorHeap)))) {
		return false;
	}
	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < frameBufferCount; ++i) {
		if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargetList[i])))) {
			return false;
		}
		device->CreateRenderTargetView(renderTargetList[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += rtvDescriptorSize;
	}

	// デプスステンシルバッファを作成.
	D3D12_CLEAR_VALUE dsClearValue = {};
	dsClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	dsClearValue.DepthStencil.Depth = 1.0f;
	dsClearValue.DepthStencil.Stencil = 0;
	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, clientWidth, clientHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&dsClearValue,
		IID_PPV_ARGS(&depthStencilBuffer)
	))) {
		return false;
	}

	// DSV用のデスクリプタヒープ及びデスクリプタを作成.
	D3D12_DESCRIPTOR_HEAP_DESC dsvDesc = {};
	dsvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDesc.NumDescriptors = 1;
	dsvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(device->CreateDescriptorHeap(&dsvDesc, IID_PPV_ARGS(&dsvDescriptorHeap)))) {
		return false;
	}
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
	device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// CBV/SRV/UAV用のデスクリプタヒープを作成.
	D3D12_DESCRIPTOR_HEAP_DESC csuDesc = {};
	csuDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	csuDesc.NumDescriptors = 1024;
	csuDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if (FAILED(device->CreateDescriptorHeap(&csuDesc, IID_PPV_ARGS(&csuDescriptorHeap)))) {
		return false;
	}
	csuDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// コマンドアロケータを作成.
	for (int i = 0; i < frameBufferCount; ++i) {
		if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i])))) {
			return false;
		}
	}

	// コマンドリストを作成.
	if (!CreateCommandList(device, commandAllocator[currentFrameIndex], commandList)) {
		return false;
	}
	if (!CreateCommandList(device, commandAllocator[currentFrameIndex], prologueCommandList)) {
		return false;
	}
	if (!CreateCommandList(device, commandAllocator[currentFrameIndex], epilogueCommandList)) {
		return false;
	}

	// フェンスとフェンスイベントを作成.
	if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) {
		return false;
	}
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (!fenceEvent) {
		return false;
	}
	for (int i = 0; i < frameBufferCount; ++i) {
		fenceValue[i] = 0;
	}
	masterFenceValue = 1;

	if (!CreatePSOList(device.Get(), warp)) {
		return false;
	}

	Resource::ResourceLoader loader;
	if (!loader.Begin(csuDescriptorHeap)) {
		return false;
	}
	if (!spriteRenderer.Init(device, frameBufferCount, 10000, loader)) {
		return false;
	}
	ID3D12CommandList* ppCommandLists[] = { loader.End() };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	WaitForGpu();

	texMap.Init(csuDescriptorHeap);

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(clientWidth);
	viewport.Height = static_cast<float>(clientHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = clientWidth;
	scissorRect.bottom = clientHeight;

	const XMMATRIX ortho = XMMatrixOrthographicLH(static_cast<float>(clientWidth), static_cast<float>(clientHeight), 1.0f, 1000.0f);
	XMStoreFloat4x4(&matViewProjection, ortho);

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

	return true;
}

/**
* 描画環境の終了処理.
*/
void Graphics::Finalize()
{
	WaitForGpu();
	CloseHandle(fenceEvent);
}

/**
* 描画開始.
*
* @retval true  描画開始処理が正常に行われた.
* @retval false 描画開始処理中にエラーが発生した.
*/
bool Graphics::BeginRendering()
{
	if (!WaitForPreviousFrame()) {
		return false;
	}

	if (FAILED(commandAllocator[currentFrameIndex]->Reset())) {
		return false;
	}

	// プロローグコマンドを作成.
	if (FAILED(prologueCommandList->Reset(commandAllocator[currentFrameIndex].Get(), nullptr))) {
		return false;
	}
	prologueCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargetList[currentFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	if (FAILED(prologueCommandList->Close())) {
		return false;
	}

	// エピローグコマンドを作成.
	if (FAILED(epilogueCommandList->Reset(commandAllocator[currentFrameIndex].Get(), nullptr))) {
		return false;
	}
	epilogueCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargetList[currentFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	if (FAILED(epilogueCommandList->Close())) {
		return false;
	}

	if (FAILED(commandList->Reset(commandAllocator[currentFrameIndex].Get(), nullptr))) {
		return false;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetDSVHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetRTVHandle();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	ID3D12DescriptorHeap* heapList[] = { csuDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(heapList), heapList);

	return true;
}

/**
* 描画終了.
*
* @retval true  描画終了処理が正常に行われた.
* @retval false 描画終了処理中にエラーが発生した.
*/
bool Graphics::EndRendering()
{
	if (FAILED(commandList->Close())) {
		return false;
	}

	ID3D12CommandList* ppCommandLists[] = { prologueCommandList.Get(), commandList.Get(), spriteRenderer.GetCommandList(), epilogueCommandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	if (FAILED(swapChain->Present(1, 0))) {
		return false;
	}
	fenceValue[currentFrameIndex] = masterFenceValue;
	if (FAILED(commandQueue->Signal(fence.Get(), masterFenceValue))) {
		return false;
	}
	++masterFenceValue;
	return true;
}

bool Graphics::WaitForPreviousFrame()
{
	if (fenceValue[currentFrameIndex] && fence->GetCompletedValue() < fenceValue[currentFrameIndex]) {
		if (FAILED(fence->SetEventOnCompletion(fenceValue[currentFrameIndex], fenceEvent))) {
			return false;
		}
		WaitForSingleObject(fenceEvent, INFINITE);
	}
	currentFrameIndex = swapChain->GetCurrentBackBufferIndex();
	return true;
}

bool Graphics::WaitForGpu()
{
	const UINT64 currentFenceValue = masterFenceValue;
	if (FAILED(commandQueue->Signal(fence.Get(), currentFenceValue))) {
		return false;
	}
	++masterFenceValue;
	if (FAILED(fence->SetEventOnCompletion(currentFenceValue, fenceEvent))) {
		return false;
	}
	WaitForSingleObject(fenceEvent, INFINITE);
	return true;
}

/**
* RTVハンドルを取得する.
*
* @return RTVデスクリプタの位置を示すD3D12_CPU_DESCRIPTOR_HANDLE型の値.
*/
D3D12_CPU_DESCRIPTOR_HANDLE Graphics::GetRTVHandle() const
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += currentFrameIndex * rtvDescriptorSize;
	return rtvHandle;
}

/**
* DSVハンドルを取得する.
*
* @return DSVデスクリプタの位置を示すD3D12_CPU_DESCRIPTOR_HANDLE型の値.
*/
D3D12_CPU_DESCRIPTOR_HANDLE Graphics::GetDSVHandle() const
{
	return dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

/**
* 頂点バッファを作成する.
*/
bool Graphics::CreateVertexBuffer()
{
	if (FAILED(device->CreateCommittedResource(
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
bool Graphics::CreateIndexBuffer()
{
	if (FAILED(device->CreateCommittedResource(
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
void Graphics::DrawTriangle()
{
	const PSO& pso = GetPSO(PSOType_Simple);
	commandList->SetPipelineState(pso.pso.Get());
	commandList->SetGraphicsRootSignature(pso.rootSignature.Get());
	commandList->SetGraphicsRootDescriptorTable(0, texNoise.handle);
	commandList->SetGraphicsRoot32BitConstants(1, 16, &matViewProjection, 0);
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->DrawInstanced(triangleVertexCount, 1, 0, 0);
}

/**
* 四角形を描画する.
*/
void Graphics::DrawRectangle()
{
	const PSO& pso = GetPSO(PSOType_NoiseTexture);
	commandList->SetPipelineState(pso.pso.Get());
	commandList->SetGraphicsRootSignature(pso.rootSignature.Get());
	commandList->SetGraphicsRootDescriptorTable(0, texBackground.handle);
	commandList->SetGraphicsRoot32BitConstants(1, 16, &matViewProjection, 0);

	static float scrollOffset = 0.0f;
	commandList->SetGraphicsRoot32BitConstants(2, 1, &scrollOffset, 0);
	scrollOffset -= 0.002f;

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);
	commandList->DrawIndexedInstanced(_countof(indices), 1, 0, triangleVertexCount, 0);
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
bool Graphics::CreateNoiseTexture(Resource::ResourceLoader& loader)
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
bool Graphics::LoadTexture()
{
	Resource::ResourceLoader loader;
	if (!loader.Begin(csuDescriptorHeap)) {
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
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	WaitForGpu();
	return true;
}

} // namespace Graphics
