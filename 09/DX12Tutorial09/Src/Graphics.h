/**
* @file Graphics.h
*/
#ifndef DX12TUTORIAL_SRC_GRAPHICS_H_
#define DX12TUTORIAL_SRC_GRAPHICS_H_
#include "Texture.h"
#include "Sprite.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
#include <wrl/client.h>

namespace Graphics {

/**
*
*/
class Graphics
{
public:
	static Graphics& Get()
	{
		static Graphics graphics;
		return graphics;
	}

	Graphics() = default;
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;

	bool Initialize(HWND hwnd, int clientWidth, int clientHeight);
	void Finalize();
	bool BeginRendering();
	bool EndRendering();
	bool WaitForPreviousFrame();
	bool WaitForGpu();

	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() const;

public:
	static const int frameBufferCount = 2;

	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetList[frameBufferCount];
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator[frameBufferCount];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> prologueCommandList;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> epilogueCommandList;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	HANDLE fenceEvent;
	UINT64 fenceValue[frameBufferCount];
	UINT64 masterFenceValue;
	int currentFrameIndex;
	int rtvDescriptorSize;
	bool warp;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;
	DirectX::XMFLOAT4X4 matViewProjection;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> csuDescriptorHeap;
	int csuDescriptorSize;

	Resource::TextureMap texMap;
	Sprite::Renderer spriteRenderer;
private:
};

} // namespace Graphics

#endif // DX12TUTORIAL_SRC_GRAPHICS_H_