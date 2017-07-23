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
* セルデータ型.
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
	DirectX::XMFLOAT3 pos; ///< スクリーン座標上のスプライトの位置.
	float rotation; ///< 画像の回転角(ラジアン).
	DirectX::XMFLOAT2 scale; ///< 画像の拡大率.
	DirectX::XMFLOAT4 color; ///< 画像の色.
};

/**
* スプライト描画情報.
*/
struct RenderingInfo
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle; ///< 描画先レンダーターゲット.
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle; ///< 描画先深度バッファ.
	D3D12_VIEWPORT viewport; ///< 描画用ビューポート.
	D3D12_RECT scissorRect; ///< 描画用シザリング矩形.
	ID3D12DescriptorHeap* texDescHeap; ///< テクスチャ用のデスクリプタヒープ.
	DirectX::XMFLOAT4X4 matViewProjection; ///< 描画に使用する座標変換行列.
};

/**
* スプライト描画クラス.
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
* セルデータの配列.
*/
struct CellList
{
	std::string name; ///< リスト名.
	std::vector<Cell> list; ///< セルデータの配列.
};

/**
* 複数のCellListをまとめたオブジェクトを操作するためのインターフェイスクラス.
*
* LoadFromJsonFile()関数を使ってインターフェイスに対応したオブジェクトを取得し、Get()で
* 個々のCellListアクセスする.
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
