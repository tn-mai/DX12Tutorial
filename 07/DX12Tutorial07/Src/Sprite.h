/**
* @file Sprite.h
*/
#pragma once
#include "Texture.h"
#include "PSO.h"
#include "Texture.h"
#include "d3dx12.h"
#include <DirectXMath.h>
#include <vector>
#include <stdint.h>

namespace Sprite {

/**
* スプライトデータ型.
*/
struct Cell {
	DirectX::XMFLOAT2 uv; ///< テクスチャ上の左上座標.
	DirectX::XMFLOAT2 tsize; ///< テクスチャ上の縦横サイズ.
	DirectX::XMFLOAT2 ssize; ///< スクリーン座標上の縦横サイズ.
};

/**
* スプライト.
*/
struct Sprite
{
	Sprite(const Cell* c, DirectX::XMFLOAT3 p, float rot, DirectX::XMFLOAT2 s, DirectX::XMFLOAT4 col);

	const Cell* cell;
	DirectX::XMFLOAT3 pos;
	float rotation;
	DirectX::XMFLOAT2 scale;
	DirectX::XMFLOAT4 color;
};

/**
* スプライト描画情報.
*/
struct RenderingInfo
{
	int frameIndex;
	std::vector<Sprite> spriteList;
	PSO pso;
	ID3D12GraphicsCommandList* commandList;
	const D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandle;
	const D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle;
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;
	ID3D12DescriptorHeap* texDescHeap;
	Resource::Texture texture;
	const void* constants;
};

/**
* スプライト描画クラス.
*/
class Renderer
{
public:
	Renderer();
	bool Init(Microsoft::WRL::ComPtr<ID3D12Device> device, int numFrameBuffer, int maxSprite, Resource::ResourceLoader& resourceLoader);
	void Draw(RenderingInfo& info);

private:
	size_t maxSpriteCount;
	int frameBufferCount;

	struct FrameResource
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		void* vertexBufferGPUAddress;
	};
	std::vector<FrameResource> frameResourceList;

	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
};

} // namespace Sprite
