/**
* @file Texture.h
*/
#ifndef DX12TUTORIAL_SRC_TEXTURE_H_
#define DX12TUTORIAL_SRC_TEXTURE_H_
#include <d3d12.h>
#include <dxgiformat.h>
#include <wrl/client.h>
#include <wincodec.h>
#include <vector>
#include <string>
#include <map>
#include <memory>

/**
* リソース管理用名前空間.
*/
namespace Resource
{

/**
* テクスチャを保持するクラス.
*/
struct Texture
{
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	DXGI_FORMAT format;
	D3D12_GPU_DESCRIPTOR_HANDLE handle;
};

/**
* リソース読み込みクラス.
*
* テクスチャの作成・読み込みの手順.
* -# ResourceLoaderオブジェクトを作成する.
* -# Beginを呼ぶ.
* -# Create, LoadFromFileでテクスチャを作成または読み込む.
* -# Endでデータ転送用コマンドリストを取得し、コマンドキューに積んで実行.
* -# ResourceLoaderオブジェクトを破棄する.
*    データ転送用バッファはこのタイミングで解放される.
*/
class ResourceLoader
{
public:
	ResourceLoader() = default;
	~ResourceLoader() = default;
	bool Begin(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap);
	ID3D12GraphicsCommandList* End();
	bool Upload(Microsoft::WRL::ComPtr<ID3D12Resource>& defaultHeap, const D3D12_RESOURCE_DESC& desc, D3D12_SUBRESOURCE_DATA data, D3D12_RESOURCE_STATES stateAfter, const wchar_t* name = nullptr);
	bool Create(Texture& texture, int index, const D3D12_RESOURCE_DESC& desc, const void* data, const wchar_t* name = nullptr);
	bool LoadFromFile(Texture& texture, int index, const wchar_t* filename);

private:
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	Microsoft::WRL::ComPtr<IWICImagingFactory> imagingFactory;
	UINT descriptorSize;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> uploadHeapList;
};

/**
* テクスチャを保持するクラス.
*
* - 指定されたテクスチャを読み込み、空きデスクリプタに割り当てる.
* - 指定されたテクスチャが既に存在する場合、そのテクスチャを返す.
*
* どこからも参照されなくなったテクスチャを破棄するため、定期的にGC()を呼び出すこと.
*/
class TextureMap
{
public:
	TextureMap() = default;
	TextureMap(const TextureMap&) = delete;
	TextureMap& operator=(const TextureMap&) = delete;

	void Init(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap);
	bool Begin();
	ID3D12GraphicsCommandList* End();
	bool Create(Texture& texture, const wchar_t* name, const D3D12_RESOURCE_DESC& desc, const void* data);
	bool LoadFromFile(Texture& texture, const wchar_t* filename);
	void ResetLoader() { loader.reset(); }
	bool Find(Texture& texture, const wchar_t* filename);
	void GC();

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	UINT descriptorSize;
	std::unique_ptr<ResourceLoader> loader;
	std::vector<uint16_t> freeIDList;

	std::map<std::wstring, Texture> map;
};

} // namespace Resource

#endif // DX12TUTORIAL_SRC_TEXTURE_H_