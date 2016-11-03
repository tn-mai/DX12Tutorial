/**
* @file Texture.cpp
*/
#include "Texture.h"
#include "d3dx12.h"
#include <wrl/client.h>
#include <wincodec.h>

namespace Texture
{

using Microsoft::WRL::ComPtr;

ComPtr<ID3D12Device> device;
ComPtr<IWICImagingFactory> imagingFactory;
ComPtr<ID3D12DescriptorHeap> descriptorHeap;
ComPtr<ID3D12GraphicsCommandList> commandList;
UINT descriptorSize;

/**
* WICフォーマットから対応するDXGIフォーマットを得る.
*
* @param wicFormatGUID WICフォーマット.
*
* @return wicFormatGUIDに対応するDXGIフォーマット.
*         対応するフォーマットが見つからない場合はDXGI_FORMAT_UNKNOWNを返す.
*/
DXGI_FORMAT GetDXGIFormatFromWICFormat(const WICPixelFormatGUID& wicFormatGUID)
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
	for (int i = 0; i < _countof(wicToDxgiList); ++i) {
		if (wicToDxgiList[i].guid == wicFormatGUID) {
			return wicToDxgiList[i].format;
		}
	}
	return DXGI_FORMAT_UNKNOWN;
}

/**
* 任意のWICフォーマットからDXGIフォーマットと互換性のあるWICフォーマットを得る.
*
* @param wicFormatGUID WICフォーマット.
*
* @return DXGIフォーマットと互換性のあるWICフォーマット.
*         元の形式をできるだけ再現できるようなフォーマットが選ばれる.
*         そのようなフォーマットが見つからない場合はGUID_WICPixelFormatDontCareを返す.
*/
WICPixelFormatGUID GetDXGICompatibleWICFormat(const WICPixelFormatGUID& wicFormatGUID)
{
	static const struct {
		WICPixelFormatGUID guid;
		WICPixelFormatGUID compatible;
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
	for (int i = 0; i < _countof(guidToCompatibleList); ++i) {
		if (guidToCompatibleList[i].guid == wicFormatGUID) {
			return guidToCompatibleList[i].compatible;
		}
	}
	return GUID_WICPixelFormatDontCare;
}

/**
* DXGIフォーマットから1ピクセルのビット数を得る.
*
* @param dxgiFormat DXGIフォーマット.
*
* @return dxgiFormatに対応するビット数.
*         対応するものがない場合は0を返す.
*/
int GetDXGIFormatBitsPerPixel(DXGI_FORMAT dxgiFormat)
{
	switch (dxgiFormat) {
	case DXGI_FORMAT_R32G32B32A32_FLOAT: return 128;
	case DXGI_FORMAT_R16G16B16A16_FLOAT: return 64;
	case DXGI_FORMAT_R16G16B16A16_UNORM: return 64;
	case DXGI_FORMAT_R8G8B8A8_UNORM: return 32;
	case DXGI_FORMAT_B8G8R8A8_UNORM: return 32;
	case DXGI_FORMAT_B8G8R8X8_UNORM: return 32;
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM: return 32;
	case DXGI_FORMAT_R10G10B10A2_UNORM: return 32;
	case DXGI_FORMAT_B5G5R5A1_UNORM: return 16;
	case DXGI_FORMAT_B5G6R5_UNORM: return 16;
	case DXGI_FORMAT_R32_FLOAT: return 32;
	case DXGI_FORMAT_R16_FLOAT: return 16;
	case DXGI_FORMAT_R16_UNORM: return 16;
	case DXGI_FORMAT_R8_UNORM: return 8;
	case DXGI_FORMAT_A8_UNORM: return 8;
	}
	return 0;
}

/**
* イメージローダーを初期化する.
*/
bool Initialize(ComPtr<ID3D12DescriptorHeap> heap, ComPtr<ID3D12GraphicsCommandList> cmdList, UINT descSize)
{
	descriptorHeap = heap;
	commandList = cmdList;
	descriptorSize = descSize;
	if (FAILED(descriptorHeap->GetDevice(IID_PPV_ARGS(&device)))) {
		return false;
	}
	if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&imagingFactory)))) {
		return false;
	}
	return true;
}

/**
* イメージローダーを破棄する.
*/
void Finalize()
{
	descriptorHeap.Reset();
	device.Reset();
	commandList.Reset();
	imagingFactory.Reset();
}

/**
* ファイルからテクスチャを読み込む.
*
* @param filename    テクスチャファイル名.
* @param imageData   テクスチャデータの読み込み先バッファ.
* @param desc        読み込んだテクスチャのリソース記述子.
* @param bytgePerRow 読み込んだテクスチャの横1列のバイト数.
*
* @retval true 読み込みに成功.
* @retval false 読み込みに失敗.
*/
bool LoadFromFile(const wchar_t* filename, std::vector<uint8_t>& imageData, D3D12_RESOURCE_DESC& desc, int& bytesPerRow)
{
	ComPtr<IWICBitmapDecoder> decoder;
	if (FAILED(imagingFactory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, decoder.GetAddressOf()))) {
		return false;
	}
	ComPtr<IWICBitmapFrameDecode> frame;
	if (FAILED(decoder->GetFrame(0, frame.GetAddressOf()))) {
		return false;
	}
	WICPixelFormatGUID pixelFormat;
	if (FAILED(frame->GetPixelFormat(&pixelFormat))) {
		return false;
	}
	UINT width, height;
	if (FAILED(frame->GetSize(&width, &height))) {
		return false;
	}
	DXGI_FORMAT dxgiFormat = GetDXGIFormatFromWICFormat(pixelFormat);
	bool imageConverted = false;
	ComPtr<IWICFormatConverter> converter;
	if (dxgiFormat == DXGI_FORMAT_UNKNOWN) {
		const WICPixelFormatGUID compatibleFormat = GetDXGICompatibleWICFormat(pixelFormat);
		if (compatibleFormat == GUID_WICPixelFormatDontCare) {
			return false;
		}
		dxgiFormat = GetDXGIFormatFromWICFormat(pixelFormat);
		if (FAILED(imagingFactory->CreateFormatConverter(converter.GetAddressOf()))) {
			return false;
		}
		BOOL canConvert = FALSE;
		if (FAILED(converter->CanConvert(pixelFormat, compatibleFormat, &canConvert))) {
			return false;
		}
		if (!canConvert) {
			return false;
		}
		if (FAILED(converter->Initialize(frame.Get(), compatibleFormat, WICBitmapDitherTypeNone, 0, 0, WICBitmapPaletteTypeCustom))) {
			return false;
		}
		imageConverted = true;
	}
	const int bitsPerPixel = GetDXGIFormatBitsPerPixel(dxgiFormat);
	bytesPerRow = (width * bitsPerPixel + 7) / 8;
	const int imageSize = bytesPerRow * height;
	imageData.resize(imageSize);
	if (imageConverted) {
		if (FAILED(converter->CopyPixels(nullptr, bytesPerRow, imageSize, imageData.data()))) {
			return false;
		}
	}
	else {
		if (FAILED(frame->CopyPixels(nullptr, bytesPerRow, imageSize, imageData.data()))) {
			return false;
		}
	}

	desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = dxgiFormat;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	return true;
}

/**
* ファイルからテクスチャを読み込む.
*
* @param descriptorIndex 読み込んだテクスチャを指すデスクリプタのインデックス.
* @param texture         読み込んだテクスチャを管理するオブジェクト.
* @param filename        テクスチャファイル名.
*
* @retval true 読み込みに成功.
* @retval false 読み込みに失敗.
*/
bool LoadFromFile(INT descriptorIndex, Texture& texture, const wchar_t* filename)
{
	if (!device || !descriptorHeap || !commandList) {
		return false;
	}

	D3D12_RESOURCE_DESC textureDesc;
	int imageBytesPerRow;
	std::vector<uint8_t> imageData;
	if (!LoadFromFile(filename, imageData, textureDesc, imageBytesPerRow)) {
		return false;
	}

	UINT64 textureHeapSize;
	device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureHeapSize);

	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&texture.resource)
	))) {
		return false;
	}
	texture.resource->SetName(filename);

	ComPtr<ID3D12Resource> uploadBuffer;
	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(textureHeapSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadBuffer)
	))) {
		return false;
	}
	D3D12_SUBRESOURCE_DATA subresource = {};
	subresource.pData = imageData.data();
	subresource.RowPitch = imageBytesPerRow;
	subresource.SlicePitch = imageBytesPerRow * textureDesc.Height;
	if (UpdateSubresources<1>(commandList.Get(), texture.resource.Get(), uploadBuffer.Get(), 0, 0, 1, &subresource) == 0) {
		return false;
	}
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture.resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(texture.resource.Get(), &srvDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), descriptorIndex, descriptorSize));

	texture.handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndex, descriptorSize);
	texture.format = textureDesc.Format;

	return true;
}

} // namespace Texture