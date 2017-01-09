/**
* @file Scene.h
*/
#ifndef DX12TUTORIAL_SRC_SCENE_H_
#define DX12TUTORIAL_SRC_SCENE_H_
#include "Texture.h"
#include "Sprite.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <windows.h>

namespace Scene {

class Stack;

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

	std::map<std::string, Resource::Texture> texMap;
	Sprite::Renderer spriteRenderer;
private:
};

/**
*
*/
class Scene {
public:
	/** The scene has a status.
	*/
	enum class StatusCode {
		Loading, ///< the scene is constructed. and it is in preparation.
		Runnable, ///< the scene is prepared. it can run.
		Pause, ///< ˆêŽž’âŽ~ó‘Ô.
		Unloading, ///< the scene is finalizing.
		Stopped, ///< the scene is finalized.it shuoud be removed as soon as possible.
	};

	explicit Scene(Graphics& e, const wchar_t* s) : env(e), status(StatusCode::Loading), name(s) {}
	virtual ~Scene() {}

	virtual bool Load() {
		status = StatusCode::Runnable;
		return true;
	}
	virtual bool Unload() {
		status = StatusCode::Stopped;
		return true;
	}
	virtual void Update(Stack&, double) = 0;
	virtual void Draw() = 0;
	virtual void Pause() {}
	virtual void Resume() {}

	StatusCode GetState() const { return status; }
	void SetState(StatusCode n) { status = n; }

private:
	Graphics& env;
	StatusCode status;
	std::wstring name;
};

typedef std::shared_ptr<Scene> ScenePtr;

/**
*
*/
class Stack
{
public:
	void Push(ScenePtr);
	ScenePtr& Top();
	const ScenePtr& Top() const { return const_cast<Stack*>(this)->Top(); }
	void Pop();
	void Repalce(ScenePtr);
	void Update(double);
	void Draw();

private:
	std::vector<ScenePtr> sceneStack;
	ScenePtr nextScene;
};

} // namespace Scene

#endif // DX12TUTORIAL_SRC_SCENE_H_