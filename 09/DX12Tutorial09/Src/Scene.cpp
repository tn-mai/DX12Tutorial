/**
* @file Scene.cpp
*/
#include "Scene.h"
#include "PSO.h"
#include "d3dx12.h"
#include <algorithm>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace Scene {

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

	return true;
}

void Graphics::Finalize()
{
	WaitForGpu();
	CloseHandle(fenceEvent);
}

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

D3D12_CPU_DESCRIPTOR_HANDLE Graphics::GetRTVHandle() const
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += currentFrameIndex * rtvDescriptorSize;
	return rtvHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE Graphics::GetDSVHandle() const
{
	return dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

/**
*
*/
bool Context::Initialize(const Transition* transitionList, size_t transitionCount, const Creator* creatorList, size_t creatorCount)
{
	transitionMap.assign(transitionList, transitionList + transitionCount);
	std::stable_sort(transitionMap.begin(), transitionMap.end(), [](const Transition& lhs, const Transition& rhs) { return lhs.currentScene < rhs.currentScene; });
	const Creator* const end = creatorList + creatorCount;
	for (const Creator* itr = creatorList; itr != end; ++itr) {
		creatorMap.insert(std::make_pair(itr->id, itr->func));
	}
	return true;
}

/**
*
*/
bool Context::Start(int startSceneId)
{
	const MapType::iterator creator = creatorMap.find(startSceneId);
	if (creator != creatorMap.end()) {
		LoadScene(creator->second);
		return true;
	}
	return false;
}

/**
*/
struct LessExitCode
{
	bool operator()(const Transition& lhs, int rhs) const { return lhs.trans.exitCode < rhs; }
	bool operator()(int lhs, const Transition& rhs) const { return lhs < rhs.trans.exitCode; }
	bool operator()(const Transition& lhs, const Transition& rhs) const { return lhs.trans.exitCode < rhs.trans.exitCode; }
};

/**
*
*/
void Context::Update(double delta)
{
	auto itrEndPausedScene = sceneStack.end() - 1;
	for (auto itr = sceneStack.begin(); itr != itrEndPausedScene; ++itr) {
		(*itr)->UpdateForPause(delta);
	}
	const int exitCode = sceneStack.back()->Update(delta);
	if (exitCode != Scene::ExitCode_Continue) {
		const auto range = std::equal_range(transitionMap.begin(), transitionMap.end(), exitCode, LessExitCode());
		const auto itr = std::find_if(range.first, range.second,
			[exitCode](const Transition& trans) { return trans.trans.exitCode == exitCode; });
		if (itr != range.second) {
			switch (itr->trans.type) {
			case TransitionType::Jump: {
				const MapType::iterator creator = creatorMap.find(itr->trans.nextScene);
				if (creator != creatorMap.end()) {
					UnloadScene();
					LoadScene(creator->second);
				}
				break;
			}
			case TransitionType::Push: {
				const MapType::iterator creator = creatorMap.find(itr->trans.nextScene);
				if (creator != creatorMap.end()) {
					sceneStack.back()->Pause();
					LoadScene(creator->second);
				}
				break;
			}
			case TransitionType::Pop:
				UnloadScene();
				sceneStack.back()->Resume();
				break;
			}
		}
	}
}

/**
*
*/
void Context::Draw(Graphics& graphics) const
{
	for (const auto& e : sceneStack) {
		if (e->GetState() == Scene::StatusCode::Runnable) {
			e->Draw(graphics);
		}
	}
}

/**
*
*/
void Context::LoadScene(Creator::Func func)
{
	sceneStack.push_back(func());
	sceneStack.back()->Load();
	sceneStack.back()->status = Scene::StatusCode::Runnable;
}

/**
*
*/
void Context::UnloadScene()
{
	sceneStack.back()->Unload();
	sceneStack.back()->status = Scene::StatusCode::Stopped;
	sceneStack.pop_back();
}

} // namespace Scene