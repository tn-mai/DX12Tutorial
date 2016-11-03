/**
* @file Texture.h
*/
#ifndef DX12TUTORIAL_SRC_TEXTURE_H_
#define DX12TUTORIAL_SRC_TEXTURE_H_
#include <d3d12.h>
#include <dxgiformat.h>
#include <wrl/client.h>
#include <vector>

/**
* テクスチャ管理用名前空間.
*/
namespace Texture
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

bool Initialize(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, UINT descSize);
void Finalize();
bool LoadFromFile(INT descriptorIndex, Texture& texture, const wchar_t* filename);

} // namespace Texture

#endif // DX12TUTORIAL_SRC_TEXTURE_H_