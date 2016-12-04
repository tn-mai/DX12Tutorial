/**
* @file Sprite.cpp
*/
#include "Sprite.h"
#include "Texture.h"
#include <algorithm>
#include <fstream>
#include <limits>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace Sprite {

namespace /* unnamed */ {

/**
* スプライト描画用頂点データ型.
*/
struct Vertex {
	constexpr Vertex(float x, float y, float z, float r, float g, float b, float a, float u, float v) : pos(x, y, z), col(r, g, b, a), texCoord(u, v) {}
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 col;
	DirectX::XMFLOAT2 texCoord;
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
		v[i].col.x = sprite.color.x;
		v[i].col.y = sprite.color.y;
		v[i].col.z = sprite.color.z;
		v[i].col.w = sprite.color.w;
		v[i].pos.z = sprite.pos.z;
	}
	v[0].pos.x = curPos.x - halfSize.x;
	v[0].pos.y = curPos.y + halfSize.y;
	v[0].texCoord.x = sprite.cell->uv.x;
	v[0].texCoord.y = sprite.cell->uv.y;
	v[1].pos.x = curPos.x + halfSize.x;
	v[1].pos.y = curPos.y - halfSize.y;
	v[1].texCoord.x = sprite.cell->uv.x + sprite.cell->tsize.x;
	v[1].texCoord.y = sprite.cell->uv.y + sprite.cell->tsize.y;
	v[2].pos.x = curPos.x - halfSize.x;
	v[2].pos.y = curPos.y - halfSize.y;
	v[2].texCoord.x = sprite.cell->uv.x;
	v[2].texCoord.y = sprite.cell->uv.y + sprite.cell->tsize.y;
	v[3].pos.x = curPos.x + halfSize.x;
	v[3].pos.y = curPos.y + halfSize.y;
	v[3].texCoord.x = sprite.cell->uv.x + sprite.cell->tsize.x;
	v[3].texCoord.y = sprite.cell->uv.y;
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
* @param sprite 描画するスプライト情報.
*/
void Renderer::Draw(RenderingInfo& info)
{
	if (info.spriteList.empty()) {
		return;
	}

	FrameResource& fr = frameResourceList[info.frameIndex];
	ID3D12GraphicsCommandList* commandList = info.commandList;

	commandList->SetGraphicsRootSignature(info.pso.rootSignature.Get());
	commandList->SetPipelineState(info.pso.pso.Get());
	ID3D12DescriptorHeap* heapList[] = { info.texDescHeap };
	commandList->SetDescriptorHeaps(_countof(heapList), heapList);
	commandList->SetGraphicsRoot32BitConstants(1, 16, info.constants, 0);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &fr.vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);
	commandList->OMSetRenderTargets(1, info.rtvHandle, FALSE, info.dsvHandle);
	commandList->RSSetViewports(1, &info.viewport);
	commandList->RSSetScissorRects(1, &info.scissorRect);

	std::stable_sort(
		info.spriteList.begin(),
		info.spriteList.end(),
		[](const Sprite& lhs, const Sprite& rhs) { return lhs.cell->texId < rhs.cell->texId; }
	);

	const XMFLOAT2 screenOffset(-(info.viewport.Width * 0.5f), info.viewport.Height * 0.5f);
	uint32_t texId = info.spriteList[0].cell->texId;
	if (texId < info.textureList.size()) {
		commandList->SetGraphicsRootDescriptorTable(0, info.textureList[texId].handle);
	}
	int numGroupSprites = 0;
	int vertexLocation = 0;
	Vertex* v = static_cast<Vertex*>(fr.vertexBufferGPUAddress);
	const Vertex* const vEnd = v + maxSpriteCount * 4;
	for (const Sprite& sprite : info.spriteList) {
		if (texId != sprite.cell->texId) {
			commandList->DrawIndexedInstanced(numGroupSprites * 6, 1, 0, vertexLocation, 0);
			vertexLocation += numGroupSprites * 6;
			numGroupSprites = 0;

			texId = sprite.cell->texId;
			if (texId < info.textureList.size()) {
				commandList->SetGraphicsRootDescriptorTable(0, info.textureList[texId].handle);
			}
		}
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
}

} // namespace Sprite
