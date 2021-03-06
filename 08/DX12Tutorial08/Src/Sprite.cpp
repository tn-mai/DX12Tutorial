/**
* @file Sprite.cpp
*/
#include "Sprite.h"
#include "Texture.h"
#include "PSO.h"
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace Sprite {

namespace /* unnamed */ {

/**
* スプライト描画用頂点データ型.
*/
struct Vertex {
	XMFLOAT3 position;
	XMFLOAT4 color;
	XMFLOAT2 texcoord;
};

XMFLOAT3 RotateZ(XMVECTOR c, float x, float y, float r)
{
	float fs, fc;
	XMScalarSinCos(&fs, &fc, r);
	const float rx = fc * x + fs * y;
	const float ry = -fs * x + fc * y;
	const XMVECTORF32 tmp{ rx, ry, 0.0f, 0.0f };
	XMFLOAT3 ret;
	XMStoreFloat3(&ret, XMVectorAdd(c, tmp));
	return ret;
}

/**
* ひとつのスプライトデータを頂点バッファに設定.
*
* @param sprite スプライトデータ.
* @param v      頂点データを描き込むアドレス.
* @param offset スクリーン左上座標.
*/
void AddVertex(const Sprite& sprite, const Cell* cell, const AnimationData& animeData, Vertex* v, XMFLOAT2 offset)
{
	const XMVECTORF32 center{ offset.x + sprite.pos.x, offset.y - sprite.pos.y, sprite.pos.z, 0.0f };
	const XMFLOAT2 halfSize{ cell->ssize.x * 0.5f * sprite.scale.x * animeData.scale.x, cell->ssize.y * 0.5f * sprite.scale.y * animeData.scale.y };

	for (int i = 0; i < 4; ++i) {
		XMStoreFloat4(&v[i].color, XMVectorMultiply(XMLoadFloat4(&sprite.color), XMLoadFloat4(&animeData.color)));
	}
	const float rotation = sprite.rotation + animeData.rotation;
	v[0].position = RotateZ(center, -halfSize.x, halfSize.y, rotation);
	v[0].texcoord.x = cell->uv.x;
	v[0].texcoord.y = cell->uv.y;

	v[1].position = RotateZ(center, halfSize.x, halfSize.y, rotation);
	v[1].texcoord.x = cell->uv.x + cell->tsize.x;
	v[1].texcoord.y = cell->uv.y;

	v[2].position = RotateZ(center, halfSize.x, -halfSize.y, rotation);
	v[2].texcoord.x = cell->uv.x + cell->tsize.x;
	v[2].texcoord.y = cell->uv.y + cell->tsize.y;

	v[3].position = RotateZ(center, -halfSize.x, -halfSize.y, rotation);
	v[3].texcoord.x = cell->uv.x;
	v[3].texcoord.y = cell->uv.y + cell->tsize.y;
}

} // unnamed namedpace

Sprite::Sprite(const AnimationList& al, DirectX::XMFLOAT3 p, float rot, DirectX::XMFLOAT2 s, DirectX::XMFLOAT4 col) :
	animeController(al),
	pos(p),
	rotation(rot),
	scale(s),
	color(col)
{
}

Renderer::Renderer() :
	maxSpriteCount(0),
	frameBufferCount(0)
{
}

/**
* Rendererを初期化する.
*
* @param device           D3Dデバイス.
* @param frameBufferCount フレームバッファの数.
* @param maxSprite        描画できる最大スプライト数.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool Renderer::Init(ComPtr<ID3D12Device> device, int numFrameBuffer, int maxSprite, Resource::ResourceLoader& resourceLoader)
{
	maxSpriteCount = maxSprite;
	frameBufferCount = numFrameBuffer;

	frameResourceList.resize(numFrameBuffer);
	for (int i = 0; i < frameBufferCount; ++i) {
		if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frameResourceList[i].commandAllocator)))) {
			return false;
		}
		if (FAILED(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(maxSpriteCount * sizeof(Vertex)),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&frameResourceList[i].vertexBuffer)
		))) {
			return false;
		}
		frameResourceList[i].vertexBuffer->SetName(L"Sprite Vertex Buffer");
		CD3DX12_RANGE range(0, 0);
		if (FAILED(frameResourceList[i].vertexBuffer->Map(0, &range, &frameResourceList[i].vertexBufferGPUAddress))) {
			return false;
		}
		frameResourceList[i].vertexBufferView.BufferLocation = frameResourceList[i].vertexBuffer->GetGPUVirtualAddress();
		frameResourceList[i].vertexBufferView.StrideInBytes = sizeof(Vertex);
		frameResourceList[i].vertexBufferView.SizeInBytes = static_cast<UINT>(maxSpriteCount * sizeof(Vertex));
	}

	if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frameResourceList[0].commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)))) {
		return false;
	}
	if (FAILED(commandList->Close())) {
		return false;
	}

	const int indexListSize = static_cast<int>(maxSpriteCount * 6 * sizeof(DWORD));
#if 1
	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(indexListSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuffer)
	))) {
		return false;
	}
	indexBuffer->SetName(L"Sprite Index Buffer");
	CD3DX12_RANGE range(0, 0);
	void* tmpIndexBufferAddress;
	if (FAILED(indexBuffer->Map(0, &range, &tmpIndexBufferAddress))) {
		return false;
	}
	DWORD* pIndexBuffer = static_cast<DWORD*>(tmpIndexBufferAddress);
	for (size_t i = 0; i < maxSpriteCount; ++i) {
		pIndexBuffer[i * 6 + 0] = i * 4 + 0;
		pIndexBuffer[i * 6 + 1] = i * 4 + 1;
		pIndexBuffer[i * 6 + 2] = i * 4 + 2;
		pIndexBuffer[i * 6 + 3] = i * 4 + 2;
		pIndexBuffer[i * 6 + 4] = i * 4 + 3;
		pIndexBuffer[i * 6 + 5] = i * 4 + 0;
	}
	indexBuffer->Unmap(0, nullptr);
#else
	std::vector<DWORD> indexList;
	indexList.resize(maxSpriteCount * 6);
	for (size_t i = 0; i < maxSpriteCount; ++i) {
		indexList[i * 6 + 0] = i * 4 + 0;
		indexList[i * 6 + 1] = i * 4 + 1;
		indexList[i * 6 + 2] = i * 4 + 2;
		indexList[i * 6 + 3] = i * 4 + 2;
		indexList[i * 6 + 4] = i * 4 + 3;
		indexList[i * 6 + 5] = i * 4 + 0;
	}

	D3D12_SUBRESOURCE_DATA subresource = { indexList.data(), indexListSize, indexListSize };
	if (!resourceLoader.Upload(indexBuffer, CD3DX12_RESOURCE_DESC::Buffer(indexListSize), subresource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, L"Sprite Index Buffer")) {
		return false;
	}
#endif
	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = indexListSize;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	return true;
}

/**
* スプライトを描画.
*
* @param spriteList 描画するスプライトのリスト.
* @param pso        描画に使用するPSO.
* @param texture    描画に使用するテクスチャ.
* @param frameIndex 現在のフレームバッファのインデックス.
* @param info       描画情報.
*
* @retval true  コマンドリスト作成成功.
* @retval false コマンドリスト作成失敗.
*/
bool Renderer::Draw(std::vector<Sprite>& spriteList, const Cell* cellList, const PSO& pso, const Resource::Texture& texture, int frameIndex, RenderingInfo& info)
{
	FrameResource& fr = frameResourceList[frameIndex];

	if (FAILED(fr.commandAllocator->Reset())) {
		return false;
	}
	if (FAILED(commandList->Reset(fr.commandAllocator.Get(), nullptr))) {
		return false;
	}
	if (spriteList.empty()) {
		return SUCCEEDED(commandList->Close());
	}

	commandList->SetGraphicsRootSignature(pso.rootSignature.Get());
	commandList->SetPipelineState(pso.pso.Get());
	ID3D12DescriptorHeap* heapList[] = { info.texDescHeap };
	commandList->SetDescriptorHeaps(_countof(heapList), heapList);
	commandList->SetGraphicsRootDescriptorTable(0, texture.handle);
	commandList->SetGraphicsRoot32BitConstants(1, 16, &info.matViewProjection, 0);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &fr.vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);
	commandList->OMSetRenderTargets(1, &info.rtvHandle, FALSE, &info.dsvHandle);
	commandList->RSSetViewports(1, &info.viewport);
	commandList->RSSetScissorRects(1, &info.scissorRect);

	const XMFLOAT2 offset(-(info.viewport.Width * 0.5f), info.viewport.Height * 0.5f);
	const int maxSprite = fr.vertexBufferView.SizeInBytes / fr.vertexBufferView.StrideInBytes / 4;
	int numSprite = 0;
	Vertex* v = static_cast<Vertex*>(fr.vertexBufferGPUAddress);
	for (const Sprite& sprite : spriteList) {
		const AnimationData& animeData = sprite.GetAnimationData();
		const Cell* cell = cellList + animeData.cellIndex;
		AddVertex(sprite, cell, animeData, v, offset);
		++numSprite;
		if (numSprite >= maxSprite) {
			break;
		}
		v += 4;
	}
	commandList->DrawIndexedInstanced(numSprite * 6, 1, 0, 0, 0);

	if (FAILED(commandList->Close())) {
		return false;
	}
	return true;
}

/**
* コマンドリストを取得する.
*
* @return ID3D12GraphicsCommandListインターフェイスへのポインタ.
*/
ID3D12GraphicsCommandList* Renderer::GetCommandList()
{
	return commandList.Get();
}

} // namespace Sprite
