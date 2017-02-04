/**
* @file Sprite.h
*/
#pragma once
#include "Animation.h"
#include "Action.h"
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
* �Z���f�[�^�^.
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
	void SetActionList(const Action::List* al) { actController.SetList(al); }
	void SetAction(uint32_t no) { actController.SetSeqIndex(no); }
	void SetCollisionId(int32_t id) { collisionId = id; }
	int32_t GetCollisionId() const { return collisionId; }
	void Update(double delta) {
		animeController.Update(delta);
		actController.Update(static_cast<float>(delta), this);
	}
	uint32_t GetCellIndex() const { return animeController.GetData().cellIndex; }
	size_t GetSeqCount() const { return animeController.GetSeqCount(); }

	AnimationController animeController;
	Action::Controller actController;
	int32_t hp;
	int32_t collisionId;
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
	bool Begin(int frameIndex);
	bool Draw(const std::vector<Sprite>& spriteList, const Cell* cellList, const PSO& pso, const Resource::Texture& texture, RenderingInfo& info);
	bool Draw(const Sprite* first, const Sprite* last, const Cell* cellList, const PSO& pso, const Resource::Texture& texture, RenderingInfo& info);
	bool End();
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
	int currentFrameIndex;
	int spriteCount;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
};

/**
* �Z���f�[�^�̔z��.
*/
struct CellList
{
	std::string name; ///< ���X�g��.
	std::vector<Cell> list; ///< �Z���f�[�^�̔z��.
};

/**
* ������CellList���܂Ƃ߂��I�u�W�F�N�g�𑀍삷�邽�߂̃C���^�[�t�F�C�X�N���X.
*
* LoadFromJsonFile()�֐����g���ăC���^�[�t�F�C�X�ɑΉ������I�u�W�F�N�g���擾���AGet()��
* �X��CellList�A�N�Z�X����.
*/
class File
{
public:
	File() = default;
	File(const File&) = delete;
	File& operator=(const File&) = delete;
	virtual ~File() {}

	virtual const CellList* Get(uint32_t no) const = 0;
	virtual size_t Size() const = 0;
};
typedef std::shared_ptr<File> FilePtr;

FilePtr LoadFromJsonFile(const wchar_t*);

} // namespace Sprite
