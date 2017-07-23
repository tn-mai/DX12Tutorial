/**
* @file Texture.cpp
*/
#include "Texture.h"
#include "d3dx12.h"

namespace Resource
{

using Microsoft::WRL::ComPtr;

/**
* WIC�t�H�[�}�b�g����Ή�����DXGI�t�H�[�}�b�g�𓾂�.
*
* @param wicFormat WIC�t�H�[�}�b�g������GUID.
*
* @return wicFormat�ɑΉ�����DXGI�t�H�[�}�b�g.
*         �Ή�����t�H�[�}�b�g��������Ȃ��ꍇ��DXGI_FORMAT_UNKNOWN��Ԃ�.
*/
DXGI_FORMAT GetDXGIFormatFromWICFormat(const WICPixelFormatGUID& wicFormat)
{
	static const struct {
		WICPixelFormatGUID guid;
		DXGI_FORMAT format;
	} wicToDxgiList[] = {
		{ GUID_WICPixelFormat128bppRGBAFloat, DXGI_FORMAT_R32G32B32A32_FLOAT },
		{ GUID_WICPixelFormat64bppRGBAHalf, DXGI_FORMAT_R16G16B16A16_FLOAT },
		{ GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM },
		{ GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM },
		{ GUID_WICPixelFormat32bppBGRA, DXGI_FORMAT_B8G8R8A8_UNORM },
		{ GUID_WICPixelFormat32bppBGR, DXGI_FORMAT_B8G8R8X8_UNORM },
		{ GUID_WICPixelFormat32bppRGBA1010102XR, DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM },
		{ GUID_WICPixelFormat32bppRGBA1010102, DXGI_FORMAT_R10G10B10A2_UNORM },
		{ GUID_WICPixelFormat16bppBGRA5551, DXGI_FORMAT_B5G5R5A1_UNORM },
		{ GUID_WICPixelFormat16bppBGR565, DXGI_FORMAT_B5G6R5_UNORM },
		{ GUID_WICPixelFormat32bppGrayFloat, DXGI_FORMAT_R32_FLOAT },
		{ GUID_WICPixelFormat16bppGrayHalf, DXGI_FORMAT_R16_FLOAT },
		{ GUID_WICPixelFormat16bppGray, DXGI_FORMAT_R16_UNORM },
		{ GUID_WICPixelFormat8bppGray, DXGI_FORMAT_R8_UNORM },
		{ GUID_WICPixelFormat8bppAlpha, DXGI_FORMAT_A8_UNORM },
	};
	for (auto e : wicToDxgiList) {
		if (e.guid == wicFormat) {
			return e.format;
		}
	}
	return DXGI_FORMAT_UNKNOWN;
}

/**
* �C�ӂ�WIC�t�H�[�}�b�g����DXGI�t�H�[�}�b�g�ƌ݊����̂���WIC�t�H�[�}�b�g�𓾂�.
*
* @param wicFormat WIC�t�H�[�}�b�g��GUID.
*
* @return DXGI�t�H�[�}�b�g�ƌ݊����̂���WIC�t�H�[�}�b�g.
*         ���̌`�����ł��邾���Č��ł���悤�ȃt�H�[�}�b�g���I�΂��.
*         ���̂悤�ȃt�H�[�}�b�g��������Ȃ��ꍇ��GUID_WICPixelFormatDontCare��Ԃ�.
*/
WICPixelFormatGUID GetDXGICompatibleWICFormat(const WICPixelFormatGUID& wicFormat)
{
	static const struct {
		WICPixelFormatGUID guid, compatible;
	} guidToCompatibleList[] = {
		{ GUID_WICPixelFormatBlackWhite, GUID_WICPixelFormat8bppGray },
		{ GUID_WICPixelFormat1bppIndexed, GUID_WICPixelFormat32bppRGBA },
		{ GUID_WICPixelFormat2bppIndexed, GUID_WICPixelFormat32bppRGBA },
		{ GUID_WICPixelFormat4bppIndexed, GUID_WICPixelFormat32bppRGBA },
		{ GUID_WICPixelFormat8bppIndexed, GUID_WICPixelFormat32bppRGBA },
		{ GUID_WICPixelFormat2bppGray, GUID_WICPixelFormat8bppGray },
		{ GUID_WICPixelFormat4bppGray, GUID_WICPixelFormat8bppGray },
		{ GUID_WICPixelFormat16bppGrayFixedPoint, GUID_WICPixelFormat16bppGrayHalf },
		{ GUID_WICPixelFormat32bppGrayFixedPoint, GUID_WICPixelFormat32bppGrayFloat },
		{ GUID_WICPixelFormat16bppBGR555, GUID_WICPixelFormat16bppBGRA5551 },
		{ GUID_WICPixelFormat32bppBGR101010, GUID_WICPixelFormat32bppRGBA1010102 },
		{ GUID_WICPixelFormat24bppBGR, GUID_WICPixelFormat32bppRGBA },
		{ GUID_WICPixelFormat24bppRGB, GUID_WICPixelFormat32bppRGBA },
		{ GUID_WICPixelFormat32bppPBGRA, GUID_WICPixelFormat32bppRGBA },
		{ GUID_WICPixelFormat32bppPRGBA, GUID_WICPixelFormat32bppRGBA },
		{ GUID_WICPixelFormat48bppRGB, GUID_WICPixelFormat64bppRGBA },
		{ GUID_WICPixelFormat48bppBGR, GUID_WICPixelFormat64bppRGBA },
		{ GUID_WICPixelFormat64bppBGRA, GUID_WICPixelFormat64bppRGBA },
		{ GUID_WICPixelFormat64bppPRGBA, GUID_WICPixelFormat64bppRGBA },
		{ GUID_WICPixelFormat64bppPBGRA, GUID_WICPixelFormat64bppRGBA },
		{ GUID_WICPixelFormat48bppRGBFixedPoint, GUID_WICPixelFormat64bppRGBAHalf },
		{ GUID_WICPixelFormat48bppBGRFixedPoint, GUID_WICPixelFormat64bppRGBAHalf },
		{ GUID_WICPixelFormat64bppRGBAFixedPoint, GUID_WICPixelFormat64bppRGBAHalf },
		{ GUID_WICPixelFormat64bppBGRAFixedPoint, GUID_WICPixelFormat64bppRGBAHalf },
		{ GUID_WICPixelFormat64bppRGBFixedPoint, GUID_WICPixelFormat64bppRGBAHalf },
		{ GUID_WICPixelFormat64bppRGBHalf, GUID_WICPixelFormat64bppRGBAHalf },
		{ GUID_WICPixelFormat48bppRGBHalf, GUID_WICPixelFormat64bppRGBAHalf },
		{ GUID_WICPixelFormat128bppPRGBAFloat, GUID_WICPixelFormat128bppRGBAFloat },
		{ GUID_WICPixelFormat128bppRGBFloat, GUID_WICPixelFormat128bppRGBAFloat },
		{ GUID_WICPixelFormat128bppRGBAFixedPoint, GUID_WICPixelFormat128bppRGBAFloat },
		{ GUID_WICPixelFormat128bppRGBFixedPoint, GUID_WICPixelFormat128bppRGBAFloat },
		{ GUID_WICPixelFormat32bppRGBE, GUID_WICPixelFormat128bppRGBAFloat },
		{ GUID_WICPixelFormat32bppCMYK, GUID_WICPixelFormat32bppRGBA },
		{ GUID_WICPixelFormat64bppCMYK, GUID_WICPixelFormat64bppRGBA },
		{ GUID_WICPixelFormat40bppCMYKAlpha, GUID_WICPixelFormat64bppRGBA },
		{ GUID_WICPixelFormat80bppCMYKAlpha, GUID_WICPixelFormat64bppRGBA },
		{ GUID_WICPixelFormat32bppRGB, GUID_WICPixelFormat32bppRGBA },
		{ GUID_WICPixelFormat64bppRGB, GUID_WICPixelFormat64bppRGBA },
		{ GUID_WICPixelFormat64bppPRGBAHalf, GUID_WICPixelFormat64bppRGBAHalf },
	};
	for (auto e : guidToCompatibleList) {
		if (e.guid == wicFormat) {
			return e.compatible;
		}
	}
	return GUID_WICPixelFormatDontCare;
}

/**
* DXGI�t�H�[�}�b�g����1�s�N�Z���̃o�C�g���𓾂�.
*
* @param dxgiFormat DXGI�t�H�[�}�b�g.
*
* @return dxgiFormat�ɑΉ�����r�b�g��.
*/
int GetDXGIFormatBitesPerPixel(DXGI_FORMAT dxgiFormat)
{
	switch (dxgiFormat) {
	case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
	case DXGI_FORMAT_R16G16B16A16_FLOAT: return 8;
	case DXGI_FORMAT_R16G16B16A16_UNORM: return 8;
	case DXGI_FORMAT_R8G8B8A8_UNORM: return 4;
	case DXGI_FORMAT_B8G8R8A8_UNORM: return 4;
	case DXGI_FORMAT_B8G8R8X8_UNORM: return 4;
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM: return 4;
	case DXGI_FORMAT_R10G10B10A2_UNORM: return 4;
	case DXGI_FORMAT_B5G5R5A1_UNORM: return 2;
	case DXGI_FORMAT_B5G6R5_UNORM: return 2;
	case DXGI_FORMAT_R32_FLOAT: return 4;
	case DXGI_FORMAT_R16_FLOAT: return 2;
	case DXGI_FORMAT_R16_UNORM: return 2;
	case DXGI_FORMAT_R8_UNORM: return 1;
	case DXGI_FORMAT_A8_UNORM: return 1;
	default: return 1;
	}
}

/**
* ���\�[�X�ǂݍ��݂��J�n����.
*
* @param heap �e�N�X�`���p��RTV�f�X�N���v�^�擾��̃f�X�N���v�^�q�[�v.
*
* @retval true  ����������.
* @retval false ���������s.
*/
bool ResourceLoader::Begin(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap)
{
	descriptorHeap = heap;
	if (FAILED(descriptorHeap->GetDevice(IID_PPV_ARGS(&device)))) {
		return false;
	}
	if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)))) {
		return false;
	}
	if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)))) {
		return false;
	}
	descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&imagingFactory)))) {
		return false;
	}
	return true;
}

/**
* ���\�[�X�ǂݍ��݂��I������.
*
* @return �R�}���h���X�g�ւ̃|�C���^.
*/
ID3D12GraphicsCommandList* ResourceLoader::End()
{
	commandList->Close();
	return commandList.Get();
}

/*
* �f�[�^���f�t�H���g�q�[�v�ɓ]������.
*/
bool ResourceLoader::Upload(Microsoft::WRL::ComPtr<ID3D12Resource>& defaultHeap, const D3D12_RESOURCE_DESC& desc, D3D12_SUBRESOURCE_DATA data, D3D12_RESOURCE_STATES stateAfter, const wchar_t* name)
{
	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&defaultHeap)
	))) {
		return false;
	}
	if (name) {
		defaultHeap->SetName(name);
	}

	UINT64 heapSize;
	device->GetCopyableFootprints(&desc, 0, 1, 0, nullptr, nullptr, nullptr, &heapSize);
	ComPtr<ID3D12Resource> uploadHeap;
	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(heapSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadHeap)
	))) {
		return false;
	}
	if (UpdateSubresources<1>(commandList.Get(), defaultHeap.Get(), uploadHeap.Get(), 0, 0, 1, &data) == 0) {
		return false;
	}
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultHeap.Get(), D3D12_RESOURCE_STATE_COPY_DEST, stateAfter));
	uploadHeapList.push_back(uploadHeap);

	return true;
}

/**
* �o�C�g�񂩂�e�N�X�`�����쐬����.
*
* @param texture  �쐬�����e�N�X�`�����Ǘ�����I�u�W�F�N�g.
* @param index    �쐬�����e�N�X�`���p��RTV�f�X�N���v�^�̃C���f�b�N�X.
* @param desc     �e�N�X�`���̏ڍ׏��.
* @param data     �e�N�X�`���쐬�Ɏg�p����o�C�g��ւ̃|�C���^.
* @param name     �e�N�X�`�����\�[�X�ɕt���閼�O(�f�o�b�O�p). nullptr��n���Ɩ��O��t���Ȃ�.
*
* @retval true  �쐬����.
* @retval false �쐬���s.
*/
bool ResourceLoader::Create(Texture& texture, int index, const D3D12_RESOURCE_DESC& desc, const void* data, const wchar_t* name)
{
	ComPtr<ID3D12Resource> textureBuffer;
	const int bytesPerRow = static_cast<int>(desc.Width * GetDXGIFormatBitesPerPixel(desc.Format));
	D3D12_SUBRESOURCE_DATA subresource = { data, bytesPerRow, static_cast<LONG_PTR>(bytesPerRow * desc.Height) };
	if (!Upload(textureBuffer, desc, subresource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, name)) {
		return false;
	}

	device->CreateShaderResourceView(textureBuffer.Get(), nullptr, CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), index, descriptorSize));

	texture.resource = textureBuffer;
	texture.format = desc.Format;
	texture.handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), index, descriptorSize);

	return true;
}

/**
* �t�@�C������e�N�X�`����ǂݍ���.
*
* @param texture   �ǂݍ��񂾃e�N�X�`�����Ǘ�����I�u�W�F�N�g.
* @param index     �ǂݍ��񂾃e�N�X�`���p��RTV�f�X�N���v�^�̃C���f�b�N�X.
* @param filename  �e�N�X�`���t�@�C����.
*
* @retval true  �ǂݍ��ݐ���.
* @retval false �ǂݍ��ݎ��s.
*/
bool ResourceLoader::LoadFromFile(Texture& texture, int index, const wchar_t* filename)
{
	ComPtr<IWICBitmapDecoder> decoder;
	if (FAILED(imagingFactory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, decoder.GetAddressOf()))) {
		return false;
	}
	ComPtr<IWICBitmapFrameDecode> frame;
	if (FAILED(decoder->GetFrame(0, frame.GetAddressOf()))) {
		return false;
	}
	WICPixelFormatGUID wicFormat;
	if (FAILED(frame->GetPixelFormat(&wicFormat))) {
		return false;
	}
	UINT width, height;
	if (FAILED(frame->GetSize(&width, &height))) {
		return false;
	}
	DXGI_FORMAT dxgiFormat = GetDXGIFormatFromWICFormat(wicFormat);
	bool imageConverted = false;
	ComPtr<IWICFormatConverter> converter;
	if (dxgiFormat == DXGI_FORMAT_UNKNOWN) {
		const WICPixelFormatGUID compatibleFormat = GetDXGICompatibleWICFormat(wicFormat);
		if (compatibleFormat == GUID_WICPixelFormatDontCare) {
			return false;
		}
		dxgiFormat = GetDXGIFormatFromWICFormat(compatibleFormat);
		if (FAILED(imagingFactory->CreateFormatConverter(converter.GetAddressOf()))) {
			return false;
		}
		BOOL canConvert = FALSE;
		if (FAILED(converter->CanConvert(wicFormat, compatibleFormat, &canConvert))) {
			return false;
		}
		if (!canConvert) {
			return false;
		}
		if (FAILED(converter->Initialize(frame.Get(), compatibleFormat, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom))) {
			return false;
		}
		imageConverted = true;
	}
	const int bytesPerRow = width * GetDXGIFormatBitesPerPixel(dxgiFormat);
	const int imageSize = bytesPerRow * height;
	std::vector<uint8_t> imageData;
	imageData.resize(imageSize);
	if (imageConverted) {
		if (FAILED(converter->CopyPixels(nullptr, bytesPerRow, imageSize, imageData.data()))) {
			return false;
		}
	} else {
		if (FAILED(frame->CopyPixels(nullptr, bytesPerRow, imageSize, imageData.data()))) {
			return false;
		}
	}

	const D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(dxgiFormat, width, height, 1, 1);
	if (!Create(texture, index, desc, imageData.data(), filename)) {
		return false;
	}
	return true;
}

/**
* ����������.
*
* @param heap �e�N�X�`���p��CSU�f�X�N���v�^�擾��̃f�X�N���v�^�q�[�v.
*/
void TextureMap::Init(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap)
{
	descriptorHeap = heap;
	size_t numDescriptors = descriptorHeap->GetDesc().NumDescriptors;
	const uint16_t avairableCount = numDescriptors > 4096 ? 4096 : static_cast<uint16_t>(numDescriptors);
	freeIDList.resize(avairableCount);
	uint16_t id = avairableCount - 1;
	for (size_t i = 0; i < avairableCount; ++i) {
		freeIDList[i] = id;
		--id;
	}
}

/**
* ���\�[�X�ǂݍ��݂��J�n����.
*
* @retval true  ����������.
* @retval false ���������s.
*/
bool TextureMap::Begin()
{
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	if (FAILED(descriptorHeap->GetDevice(IID_PPV_ARGS(&device)))) {
		return false;
	}
	descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	loader.reset(new ResourceLoader);
	loader->Begin(descriptorHeap);
	return true;
}

/**
* ���\�[�X�ǂݍ��݂��I������.
*
* @return �R�}���h���X�g�ւ̃|�C���^.
*/
ID3D12GraphicsCommandList* TextureMap::End()
{
	return loader->End();
}

/**
* ���O����v����e�N�X�`������������.
*
* @param texture  �߂�l��true�̏ꍇ�ɁA���������e�N�X�`�����i�[����I�u�W�F�N�g.
* @param name     ��������e�N�X�`����.
*
* @retval true  name�Ɉ�v����e�N�X�`���𔭌�.
* @retval false name�Ɉ�v����e�N�X�`����ێ����Ă��Ȃ�.
*/
bool TextureMap::Find(Texture& texture, const wchar_t* name)
{
	auto itr = map.find(name);
	if (itr == map.end()) {
		return false;
	}
	texture = itr->second;
	return true;
}

/**
* �o�C�g�񂩂�e�N�X�`�����쐬����.
*
* @param texture  �쐬�����e�N�X�`�����Ǘ�����I�u�W�F�N�g.
* @param name     �e�N�X�`�����\�[�X�ɕt���閼�O.
* @param desc     �e�N�X�`���̏ڍ׏��.
* @param data     �e�N�X�`���쐬�Ɏg�p����o�C�g��ւ̃|�C���^.
*
* @retval true  �쐬����.
* @retval false �쐬���s.
*/
bool TextureMap::Create(Texture& texture, const wchar_t* name, const D3D12_RESOURCE_DESC& desc, const void* data)
{
	if (Find(texture, name)) {
		return true;
	}

	if (freeIDList.empty()) {
		return false;
	}
	const int index = freeIDList.back();
	if (loader->Create(texture, index, desc, data, name)) {
		freeIDList.pop_back();
		map.insert(std::make_pair(name, texture));
		return true;
	}
	return false;
}

/**
* �t�@�C������e�N�X�`����ǂݍ���.
*
* @param texture   �ǂݍ��񂾃e�N�X�`�����Ǘ�����I�u�W�F�N�g.
* @param filename  �e�N�X�`���t�@�C����.
*
* @retval true  �ǂݍ��ݐ���.
* @retval false �ǂݍ��ݎ��s.
*/
bool TextureMap::LoadFromFile(Texture& texture, const wchar_t* filename)
{
	if (Find(texture, filename)) {
		return true;
	}

	if (freeIDList.empty()) {
		return false;
	}
	const int index = freeIDList.back();
	if (loader->LoadFromFile(texture, index, filename)) {
		freeIDList.pop_back();
		map.insert(std::make_pair(filename, texture));
		return true;
	}
	return false;
}

/**
* �Q�Ƃ���Ȃ��Ȃ����e�N�X�`����j������.
*/
void TextureMap::GC()
{
	auto itr = map.begin();
	while (itr != map.end()) {
		itr->second.resource.Get()->AddRef();
		const ULONG refCount = itr->second.resource.Get()->Release();
		if (refCount == 1) {
			const int index = static_cast<int>((itr->second.handle.ptr - descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr) / descriptorSize);
			itr = map.erase(itr);
			freeIDList.push_back(index);
		} else {
			++itr;
		}
	}
}

} // namespace Resource