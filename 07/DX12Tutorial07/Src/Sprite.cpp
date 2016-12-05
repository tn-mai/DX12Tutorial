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

/**
* ひとつのスプライトデータを頂点バッファに設定.
*/
void AddVertex(const Sprite& sprite, volatile Vertex* v, XMFLOAT2 screenOffset)
{
	// 0-3
	// |\|
	// 2-1
	const DirectX::XMFLOAT2 curPos(screenOffset.x + sprite.pos.x, screenOffset.y - sprite.pos.y);
	DirectX::XMFLOAT2 halfSize(sprite.cell->ssize.x * 0.5f * sprite.scale.x, sprite.cell->ssize.y * 0.5f * sprite.scale.y);

	for (int i = 0; i < 4; ++i) {
		v[i].color.x = sprite.color.x;
		v[i].color.y = sprite.color.y;
		v[i].color.z = sprite.color.z;
		v[i].color.w = sprite.color.w;
		v[i].position.z = sprite.pos.z;
	}
	v[0].position.x = curPos.x - halfSize.x;
	v[0].position.y = curPos.y + halfSize.y;
	v[0].texcoord.x = sprite.cell->uv.x;
	v[0].texcoord.y = sprite.cell->uv.y;
	v[1].position.x = curPos.x + halfSize.x;
	v[1].position.y = curPos.y - halfSize.y;
	v[1].texcoord.x = sprite.cell->uv.x + sprite.cell->tsize.x;
	v[1].texcoord.y = sprite.cell->uv.y + sprite.cell->tsize.y;
	v[2].position.x = curPos.x - halfSize.x;
	v[2].position.y = curPos.y - halfSize.y;
	v[2].texcoord.x = sprite.cell->uv.x;
	v[2].texcoord.y = sprite.cell->uv.y + sprite.cell->tsize.y;
	v[3].position.x = curPos.x + halfSize.x;
	v[3].position.y = curPos.y + halfSize.y;
	v[3].texcoord.x = sprite.cell->uv.x + sprite.cell->tsize.x;
	v[3].texcoord.y = sprite.cell->uv.y;
}

} // unnamed namedpace

Sprite::Sprite(const Cell* c, DirectX::XMFLOAT3 p, float rot, DirectX::XMFLOAT2 s, DirectX::XMFLOAT4 col) :
	cell(c),
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
			IID_PPV_ARGS(frameResourceList[i].vertexBuffer.GetAddressOf())
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

	std::vector<DWORD> indexList;
	indexList.resize(maxSpriteCount * 6);
	for (size_t i = 0; i < maxSpriteCount; ++i) {
		indexList[i * 6 + 0] = i * 4 + 0;
		indexList[i * 6 + 1] = i * 4 + 1;
		indexList[i * 6 + 2] = i * 4 + 2;
		indexList[i * 6 + 3] = i * 4 + 0;
		indexList[i * 6 + 4] = i * 4 + 3;
		indexList[i * 6 + 5] = i * 4 + 1;
	}
	const int indexListSize = static_cast<int>(maxSpriteCount * 6 * sizeof(DWORD));
	D3D12_SUBRESOURCE_DATA subresource = { indexList.data(), indexListSize, indexListSize };
	if (!resourceLoader.Upload(indexBuffer, CD3DX12_RESOURCE_DESC::Buffer(indexListSize), subresource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, L"Sprite Index Buffer")) {
		return false;
	}
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
bool Renderer::Draw(std::vector<Sprite> spriteList, const PSO& pso, const Resource::Texture& texture, int frameIndex, RenderingInfo& info)
{
	FrameResource& fr = frameResourceList[frameIndex];

	if (FAILED(fr.commandAllocator->Reset())) {
		return false;
	}

	if (FAILED(commandList->Reset(fr.commandAllocator.Get(), nullptr))) {
		return false;
	}

	if (spriteList.empty()) {
		if (FAILED(commandList->Close())) {
			return false;
		}
		return true;
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

	const XMFLOAT2 screenOffset(-(info.viewport.Width * 0.5f), info.viewport.Height * 0.5f);
	int numGroupSprites = 0;
	int vertexLocation = 0;
	Vertex* v = static_cast<Vertex*>(fr.vertexBufferGPUAddress);
	const Vertex* const vEnd = v + maxSpriteCount * 4;
	for (const Sprite& sprite : spriteList) {
		AddVertex(sprite, v, screenOffset);
		++numGroupSprites;
		v += 4;
		if (v >= vEnd) {
			break;
		}
	}
	if (numGroupSprites) {
		commandList->DrawIndexedInstanced(numGroupSprites * 6, 1, 0, vertexLocation, 0);
	}

	if (FAILED(commandList->Close())) {
		return false;
	}
	return true;
}

} // namespace Sprite
