/**
* @file Sprite.h
*/
#pragma once
#include "Animation.h"
#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>

namespace Resource {
struct Texture;
class ResourceLoader;
}
struct PSO;

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
	Sprite() = delete;
	Sprite(const AnimationList& al, DirectX::XMFLOAT3 p, float rot = 0, DirectX::XMFLOAT2 s = DirectX::XMFLOAT2(1, 1), DirectX::XMFLOAT4 col = DirectX::XMFLOAT4(1, 1, 1, 1));
	void SetSeqIndex(uint32_t no) { animeController.SetSeqIndex(no); }
	void Update(double delta) { animeController.Update(delta); }
	uint32_t GetCellIndex() const { return animeController.GetCellIndex(); }
	size_t GetSeqCount() const { return animeController.GetSeqCount(); }

	AnimationController animeController;
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
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle; ///< �`��惌���_�[�^�[�Q�b�g.
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle; ///< �`���[�x�o�b�t�@.
	D3D12_VIEWPORT viewport; ///< �`��p�r���[�|�[�g.
	D3D12_RECT scissorRect; ///< �`��p�V�U�����O��`.
	ID3D12DescriptorHeap* texDescHeap; ///< �e�N�X�`���p�̃f�X�N���v�^�q�[�v.
	DirectX::XMFLOAT4X4 matViewProjection; ///< �`��Ɏg�p������W�ϊ��s��.
};

/**
* �X�v���C�g�`��N���X.
*/
class Renderer
{
public:
	Renderer();
	bool Init(Microsoft::WRL::ComPtr<ID3D12Device> device, int numFrameBuffer, int maxSprite, Resource::ResourceLoader& resourceLoader);
	bool Draw(std::vector<Sprite>& spriteList, const Cell* cellList, const PSO& pso, const Resource::Texture& texture, int frameIndex, RenderingInfo& info);
	ID3D12GraphicsCommandList* GetCommandList();

private:
	size_t maxSpriteCount;
	int frameBufferCount;

	struct FrameResource
	{
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		void* vertexBufferGPUAddress;
	};
	std::vector<FrameResource> frameResourceList;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
};

} // namespace Sprite