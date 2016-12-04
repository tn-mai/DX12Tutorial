/**
* @file Texture.cpp
*/
#include "Texture.h"
#include "d3dx12.h"

namespace Texture
{

using Microsoft::WRL::ComPtr;

/**
* WICフォーマットから対応するDXGIフォーマットを得る.
*
* @param wicFormat WICフォーマットを示すGUID.
*
* @return wicFormatに対応するDXGIフォーマット.
*         対応するフォーマットが見つからない場合はDXGI_FORMAT_UNKNOWNを返す.
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
* 任意のWICフォーマットからDXGIフォーマットと互換性のあるWICフォーマットを得る.
*
* @param wicFormat WICフォーマットのGUID.
*
* @return DXGIフォーマットと互換性のあるWICフォーマット.
*         元の形式をできるだけ再現できるようなフォーマットが選ばれる.
*         そのようなフォーマットが見つからない場合はGUID_WICPixelFormatDontCareを返す.
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
* DXGIフォーマットから1ピクセルのバイト数を得る.
*
* @param dxgiFormat DXGIフォーマット.
*
* @return dxgiFormatに対応するビット数.
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
* テクスチャ読み込みを開始する.
*
* @param heap テクスチャ用のRTVデスクリプタ取得先のデスクリプタヒープ.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool TextureLoader::Begin(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap)
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
* テクスチャ読み込みを終了する.
*
* @return コマンドリストへのポインタ.
*/
ID3D12GraphicsCommandList* TextureLoader::End()
{
	commandList->Close();
	return commandList.Get();
}

/*
* データをデフォルトヒープに転送する.
*/
bool TextureLoader::Upload(Microsoft::WRL::ComPtr<ID3D12Resource>& defaultHeap, const D3D12_RESOURCE_DESC& desc, D3D12_SUBRESOURCE_DATA data, D3D12_RESOURCE_STATES stateAfter, const wchar_t* name)
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
* バイト列からテクスチャを作成する.
*
* @param texture  作成したテクスチャを管理するオブジェクト.
* @param index    作成したテクスチャ用のRTVデスクリプタのインデックス.
* @param desc     テクスチャの詳細情報.
* @param data     テクスチャ作成に使用するバイト列へのポインタ.
* @param name     テクスチャリソースに付ける名前(デバッグ用). nullptrを渡すと名前を付けない.
*
* @retval true  作成成功.
* @retval false 作成失敗.
*/
bool TextureLoader::Create(Texture& texture, int index, const D3D12_RESOURCE_DESC& desc, const void* data, const wchar_t* name)
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
* ファイルからテクスチャを読み込む.
*
* @param texture   読み込んだテクスチャを管理するオブジェクト.
* @param index     読み込んだテクスチャ用のRTVデスクリプタのインデックス.
* @param filename  テクスチャファイル名.
*
* @retval true  読み込み成功.
* @retval false 読み込み失敗.
*/
bool TextureLoader::LoadFromFile(Texture& texture, int index, const wchar_t* filename)
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

} // namespace Texture