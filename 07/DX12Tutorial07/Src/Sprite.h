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
* �X�v���C�g�f�[�^�^.
*/
struct Cell {
	DirectX::XMFLOAT2 uv; ///< �e�N�X�`����̍�����W.
	DirectX::XMFLOAT2 tsize; ///< �e�N�X�`����̏c���T�C�Y.
	DirectX::XMFLOAT2 ssize; ///< �X�N���[�����W��̏c���T�C�Y.
};

/**
* �X�v���C�g.
*/
struct Sprite
{
	Sprite(const Cell* c, DirectX::XMFLOAT3 p, float rot, DirectX::XMFLOAT2 s, DirectX::XMFLOAT4 col);

	const Cell* cell; ///< �\������Cell�f�[�^.
	DirectX::XMFLOAT3 pos; ///< �X�N���[�����W��̃X�v���C�g�̈ʒu.
	float rotation; ///< �摜�̉�]�p(���W�A��).
	DirectX::XMFLOAT2 scale; ///< �摜�̊g�嗦.
	DirectX::XMFLOAT4 color; ///< �摜�̐F.
};

/**
* �X�v���C�g�`����.
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
* �X�v���C�g�`��N���X.
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
