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
* ���\�[�X�Ǘ��p���O���.
*/
namespace Resource
{

/**
* �e�N�X�`����ێ�����N���X.
*/
struct Texture
{
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	DXGI_FORMAT format;
	D3D12_GPU_DESCRIPTOR_HANDLE handle;
};

/**
* ���\�[�X�ǂݍ��݃N���X.
*
* �e�N�X�`���̍쐬�E�ǂݍ��݂̎菇.
* -# ResourceLoader�I�u�W�F�N�g���쐬����.
* -# Begin���Ă�.
* -# Create, LoadFromFile�Ńe�N�X�`�����쐬�܂��͓ǂݍ���.
* -# End�Ńf�[�^�]���p�R�}���h���X�g���擾���A�R�}���h�L���[�ɐς�Ŏ��s.
* -# ResourceLoader�I�u�W�F�N�g��j������.
*    �f�[�^�]���p�o�b�t�@�͂��̃^�C�~���O�ŉ�������.
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
* �e�N�X�`����ێ�����N���X.
*
* - �w�肳�ꂽ�e�N�X�`����ǂݍ��݁A�󂫃f�X�N���v�^�Ɋ��蓖�Ă�.
* - �w�肳�ꂽ�e�N�X�`�������ɑ��݂���ꍇ�A���̃e�N�X�`����Ԃ�.
*
* �ǂ�������Q�Ƃ���Ȃ��Ȃ����e�N�X�`����j�����邽�߁A����I��GC()���Ăяo������.
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