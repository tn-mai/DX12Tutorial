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

class Context;

/**
*
*/
class Scene
{
	friend class Context;

public:
	/** The scene has a status.
	*/
	enum class StatusCode {
		Loading, ///< the scene is constructed. and it is in preparation.
		Runnable, ///< the scene is prepared. it can run.
		Unloading, ///< the scene is finalizing.
		Stopped, ///< the scene is finalized.it shuoud be removed as soon as possible.
	};

	enum ExitCode {
		ExitCode_Continue = -2,
		ExitCode_Exit = -1,
		ExitCode_User = 0,
	};

	explicit Scene(const wchar_t* s) : status(StatusCode::Loading), name(s) {}
	virtual ~Scene() {}

	StatusCode GetState() const { return status; }

private:
	virtual bool Load() { return true; }
	virtual bool Unload() { return true; }
	virtual int Update(double) = 0;
	virtual void UpdateForPause(double) {}
	virtual void Draw(Graphics&) const = 0;
	virtual void Pause() {}
	virtual void Resume() {}

private:
	StatusCode status;
	std::wstring name;
};

typedef std::shared_ptr<Scene> ScenePtr;

struct Creator
{
	typedef ScenePtr(*Func)();

	int id;
	Func func;
};

enum class TransitionType
{
	Jump,
	Push,
	Pop,
};

struct Transition
{
	int currentScene;
	struct {
		int exitCode;
		TransitionType type;
		int nextScene;
	} trans;
};

/**
*
*/
class Context
{
public:
	bool Initialize(const Transition* transitionList, size_t transitionCount, const Creator* creatorList, size_t creatorCount);
	bool Start(int);
	void Update(double delta);
	void Draw(Graphics&) const;

private:
	void LoadScene(Creator::Func);
	void UnloadScene();

private:
	typedef std::vector<Transition> TransitionMap;
	typedef std::map<int, Creator::Func> MapType;

	MapType creatorMap;
	TransitionMap transitionMap;

	std::vector<ScenePtr> sceneStack;
};

} // namespace Scene

#endif // DX12TUTORIAL_SRC_SCENE_H_