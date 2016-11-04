/**
* @file Engine.cpp
*/
#include "Engine.h"
#include "d3dx12.h"
#include <exception>

/**
* COMメソッドの結果が「失敗」だった場合、例外を送出する.
*
* @param hr COMメソッドの結果を表すHRESULT型の値.
*/
inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr)) {
		throw std::exception();
	}
}

/**
* ゲームエンジンを初期化する.
*
* @param hwnd         描画先のウィンドウハンドル.
* @param clientWidth  ウィンドウの描画領域の幅.
* @param clientHeight ウィンドウの描画領域の高さ.
*/
void Engine::Initialize(HWND hwnd, int clientWidth, int clientHeight)
{
	IDXGIFactory4* dxgiFactory;
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

	// 機能レベル11を満たすハードウェアアダプタを検索し、そのデバイスインターフェイスを取得する.
	IDXGIAdapter1* dxgiAdapter;
	int adapterIndex = 0;
	bool adapterFound = false;
	while (dxgiFactory->EnumAdapters1(adapterIndex, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC1 desc;
		dxgiAdapter->GetDesc1(&desc);
		if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
			const HRESULT hr = D3D12CreateDevice(dxgiAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
			if (SUCCEEDED(hr)) {
				adapterFound = true;
				break;
			}
		}
		++adapterIndex;
	}
	// 機能レベル11を満たすハードウェアが見つからない場合、WARPデバイスの作成を試みる.
	// WARP=(Windows Advanced Rasterization Platform)
	warp = false;
	if (!adapterFound) {
		ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter)));
		warp = true;
	}
	ThrowIfFailed(D3D12CreateDevice(dxgiAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device.GetAddressOf())));

	// コマンドキューを作成.
	// 通常、コマンドキューはデバイスごとに1つだけ作成する.
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	ThrowIfFailed(device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(commandQueue.GetAddressOf())));

	// スワップチェーンを作成.
	const int frameBufferCount = 3;
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = clientWidth;
	scDesc.Height = clientHeight;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.SampleDesc.Count = 1;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = frameBufferCount;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	{
		ComPtr<IDXGISwapChain1> tmpSwapChain;
		ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &scDesc, nullptr, nullptr, tmpSwapChain.GetAddressOf()));
		tmpSwapChain.As(&swapChain);
	}
	currentFrameIndex = swapChain->GetCurrentBackBufferIndex();

	// RTV用のデスクリプタヒープ及びデスクリプタを作成.
	D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
	rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDesc.NumDescriptors = frameBufferCount;
	rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(rtvDescriptorHeap.GetAddressOf())));
	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < frameBufferCount; ++i) {
		ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(renderTargetList[i].GetAddressOf())));
		device->CreateRenderTargetView(renderTargetList[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += rtvDescriptorSize;
	}

	// DS用のデスクリプタヒープ及びデスクリプタを作成.
	D3D12_DESCRIPTOR_HEAP_DESC dsvDesc = {};
	dsvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDesc.NumDescriptors = 1;
	dsvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(device->CreateDescriptorHeap(&dsvDesc, IID_PPV_ARGS(dsvDescriptorHeap.GetAddressOf())));
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
	D3D12_CLEAR_VALUE dsClearValue = {};
	dsClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	dsClearValue.DepthStencil.Depth = 1.0f;
	dsClearValue.DepthStencil.Stencil = 0;
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, clientWidth, clientHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&dsClearValue,
		IID_PPV_ARGS(depthStencilBuffer.GetAddressOf())
	));
	device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

/**
* ゲームエンジンの状態を更新する.
*/
void Engine::Update()
{
}

/**
* 描画する.
*/
void Engine::Render()
{
//	ThrowIfFailed(commandAllocator->Reset());
//	ThrowIfFailed(commandList->Reset(commandAllocator[0].Get(), pso.Get()));
}

/**
* ゲームエンジンを終了する.
*/
void Engine::Finalize()
{
}

void Engine::WaitForGpu()
{
}

void Engine::MoveToNextFrame()
{
}