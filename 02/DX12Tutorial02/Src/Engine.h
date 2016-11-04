/**
* @file Engine.h
*/
#ifndef DX12TUTORIAL_SRC_ENGINE_H_
#define DX12TUTORIAL_SRC_ENGINE_H_
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

class Engine
{
public:
	Engine() {}
	void Initialize(HWND, int, int);
	void Update();
	void Render();
	void Finalize();

private:
	void WaitForGpu();
	void MoveToNextFrame();

	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	static const int frameBufferCount = 3;

	ComPtr<ID3D12Device> device;
	ComPtr<IDXGISwapChain3> swapChain;
	ComPtr<ID3D12CommandQueue> commandQueue;
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
	ComPtr<ID3D12Resource> renderTargetList[frameBufferCount];
	ComPtr<ID3D12Resource> depthStencilBuffer;
	ComPtr<ID3D12CommandAllocator> commandAllocator[frameBufferCount];
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ComPtr<ID3D12Fence> fence[frameBufferCount];
	HANDLE fenceEvent;
	UINT64 fenceValue[frameBufferCount];
	int currentFrameIndex;
	int rtvDescriptorSize;
	bool warp;
};

#endif // DX12TUTORIAL_SRC_ENGINE_H_