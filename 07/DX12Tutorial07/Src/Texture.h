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

/**
* �e�N�X�`���Ǘ��p���O���.
*/
namespace Texture
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

} // namespace Texture

#endif // DX12TUTORIAL_SRC_TEXTURE_H_