/**
* @file Sprite.cpp
*/
#include "Sprite.h"
#include "Texture.h"
#include "PSO.h"
#include "Json.h"
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
void AddVertex(const Sprite& sprite, const Cell* cell, const AnimationData& anm, Vertex* v, XMFLOAT2 offset)
{
	const XMVECTORF32 center{ offset.x + sprite.pos.x, offset.y - sprite.pos.y, sprite.pos.z, 0.0f };
	const XMFLOAT2 halfSize{ cell->ssize.x * 0.5f * sprite.scale.x * anm.scale.x, cell->ssize.y * 0.5f * sprite.scale.y * anm.scale.y };

	const XMVECTOR vcolor = XMVectorMultiply(XMLoadFloat4(&sprite.color), XMLoadFloat4(&anm.color));
	for (int i = 0; i < 4; ++i) {
		XMStoreFloat4(&v[i].color, vcolor);
	}
	const float rot = sprite.rotation + anm.rotation;
	v[0].position = RotateZ(center, -halfSize.x, halfSize.y, rot);
	v[0].texcoord.x = cell->uv.x;
	v[0].texcoord.y = cell->uv.y;

	v[1].position = RotateZ(center, halfSize.x, halfSize.y, rot);
	v[1].texcoord.x = cell->uv.x + cell->tsize.x;
	v[1].texcoord.y = cell->uv.y;

	v[2].position = RotateZ(center, halfSize.x, -halfSize.y, rot);
	v[2].texcoord.x = cell->uv.x + cell->tsize.x;
	v[2].texcoord.y = cell->uv.y + cell->tsize.y;

	v[3].position = RotateZ(center, -halfSize.x, -halfSize.y, rot);
	v[3].texcoord.x = cell->uv.x;
	v[3].texcoord.y = cell->uv.y + cell->tsize.y;
}

} // unnamed namedpace

Sprite::Sprite(const AnimationList& al, DirectX::XMFLOAT3 p, float rot, DirectX::XMFLOAT2 s, DirectX::XMFLOAT4 col) :
	animeController(al),
	actController(),
	collisionId(-1),
	pos(p),
	rotation(rot),
	scale(s),
	color(col)
{
}

Renderer::Renderer() :
	maxSpriteCount(0),
	frameBufferCount(0),
	currentFrameIndex(-1)
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
* スプライトの描画開始.
*
* @param frameIndex 現在のフレームバッファのインデックス.
*
* @retval true 描画可能な状態になった.
* @retval false 描画可能な状態への遷移に失敗.
*/
bool Renderer::Begin(int frameIndex)
{
	if (currentFrameIndex >= 0) {
		return false;
	}

	currentFrameIndex = frameIndex;
	FrameResource& fr = frameResourceList[currentFrameIndex];

	if (FAILED(fr.commandAllocator->Reset())) {
		return false;
	}
	if (FAILED(commandList->Reset(fr.commandAllocator.Get(), nullptr))) {
		return false;
	}
	spriteCount = 0;
	return true;
}

/**
* スプライトを描画.
*
* @param spriteList 描画するスプライトのリスト.
* @param pso        描画に使用するPSO.
* @param texture    描画に使用するテクスチャ.
* @param info       描画情報.
*
* @retval true  コマンドリスト作成成功.
* @retval false コマンドリスト作成失敗.
*/
bool Renderer::Draw(const std::vector<Sprite>& spriteList, const Cell* cellList, const PSO& pso, const Resource::Texture& texture, RenderingInfo& info)
{
	if (spriteList.empty()) {
		return true;
	}
	return Draw(&*spriteList.begin(), (&*spriteList.begin()) + spriteList.size(), cellList, pso, texture, info);
}

bool Renderer::Draw(const Sprite* first, const Sprite* last, const Cell* cellList, const PSO& pso, const Resource::Texture& texture, RenderingInfo& info)
{
	if (currentFrameIndex < 0) {
		return false;
	}
	if (first == last) {
		return true;
	}

	FrameResource& fr = frameResourceList[currentFrameIndex];

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
	const int remainingSprite = (fr.vertexBufferView.SizeInBytes / fr.vertexBufferView.StrideInBytes / 4) - spriteCount;
	int numSprite = 0;
	Vertex* v = static_cast<Vertex*>(fr.vertexBufferGPUAddress) + (spriteCount * 4);
	for (const Sprite* sprite = first; sprite != last; ++sprite) {
		if (sprite->scale.x == 0 || sprite->scale.y == 0) {
			continue;
		}
		const Cell* cell = cellList + sprite->GetCellIndex();
		AddVertex(*sprite, cell, sprite->animeController.GetData(), v, offset);
		++numSprite;
		if (numSprite >= remainingSprite) {
			break;
		}
		v += 4;
	}
	commandList->DrawIndexedInstanced(numSprite * 6, 1, 0, spriteCount * 4, 0);
	spriteCount += numSprite;

	return true;
}

/**
* スプライトの描画終了.
*
* @retval true  コマンドリスト作成成功.
* @retval false コマンドリスト作成失敗.
*/
bool Renderer::End()
{
	currentFrameIndex = -1;
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

/**
* Fileインターフェイスの実装クラス.
*/
class FileImpl : public File
{
public:
	FileImpl() {}
	virtual ~FileImpl() {}
	virtual const CellList* Get(uint32_t no) const {
		if (no >= clList.size()) {
			return nullptr;
		}
		return &clList[no];
	}
	virtual size_t Size() const { return clList.size(); }

	std::vector<CellList> clList;
};

/**
* ファイルからセルリストを読み込む.
*
* @param filename ファイル名.
*
* @return 読み込んだセルリスト.
*         読み込み失敗の場合はnullptrを返す.
*
* JSONフォーマットは次のとおり:
* <pre>
* [
*   {
*     "name" : "セルリスト名",
*     "texsize" : [w, h],
*     "list" : [
*       {
*         "uv" : [u, v],
*         "tsize" : [tw, th],
*         "ssize" : [sw, sh]
*       },
*       ...
*     ]
*   },
*   ...
* ]
* </pre>
*/
FilePtr LoadFromJsonFile(const wchar_t* filename)
{
	struct HandleHolder {
		explicit HandleHolder(HANDLE h) : handle(h) {}
		~HandleHolder() { if (handle != INVALID_HANDLE_VALUE) { CloseHandle(handle); } }
		HANDLE handle;
		operator HANDLE() { return handle; }
		operator HANDLE() const { return handle; }
	};

	std::shared_ptr<FileImpl> af(new FileImpl);

	HandleHolder h(CreateFileW(filename, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
	if (h == INVALID_HANDLE_VALUE) {
		return af;
	}
	LARGE_INTEGER size;
	if (!GetFileSizeEx(h, &size)) {
		return af;
	}
	if (size.QuadPart > std::numeric_limits<size_t>::max()) {
		return af;
	}
	std::vector<char> buffer;
	buffer.resize(static_cast<size_t>(size.QuadPart));
	DWORD readBytes;
	if (!ReadFile(h, &buffer[0], buffer.size(), &readBytes, nullptr)) {
		return af;
	}
	const Json::Value json = Json::Parse(buffer.data());
	if (json.type != Json::Type::Array) {
		return af;
	}

	for (const Json::Value& e : json.array) {
		if (e.type != Json::Type::Object) {
			break;
		}
		const Json::Object::const_iterator itrName = e.object.find("name");
		if (itrName == e.object.end() || itrName->second.type != Json::Type::String) {
			break;
		}
		const Json::Object::const_iterator itrTexSize = e.object.find("texsize");
		if (itrTexSize == e.object.end() || itrTexSize->second.type != Json::Type::Array || itrTexSize->second.array.size() < 2) {
			break;
		}
		const XMVECTOR texsize = XMVectorReciprocal({ static_cast<float>(itrTexSize->second.array[0].number), static_cast<float>(itrTexSize->second.array[1].number) });
		CellList al;
		al.name = itrName->second.string;
		const Json::Object::const_iterator itrList = e.object.find("list");
		if (itrList == e.object.end() || itrList->second.type != Json::Type::Array) {
			break;
		}
		for (const Json::Value& data : itrList->second.array) {
			if (data.type != Json::Type::Object) {
				return af;
			}
			Cell cell;
			const Json::Array& uv = data.object.find("uv")->second.array;
			cell.uv.x = uv.size() > 0 ? static_cast<float>(uv[0].number) : 0.0f;
			cell.uv.y = uv.size() > 1 ? static_cast<float>(uv[1].number) : 0.0f;
			XMStoreFloat2(&cell.uv, XMVectorMultiply(XMLoadFloat2(&cell.uv), texsize));
			const Json::Array& tsize = data.object.find("tsize")->second.array;
			cell.tsize.x = tsize.size() > 0 ? static_cast<float>(tsize[0].number) : 0.0f;
			cell.tsize.y = tsize.size() > 1 ? static_cast<float>(tsize[1].number) : 0.0f;
			XMStoreFloat2(&cell.tsize, XMVectorMultiply(XMLoadFloat2(&cell.tsize), texsize));
			const Json::Array& ssize = data.object.find("ssize")->second.array;
			cell.ssize.x = ssize.size() > 0 ? static_cast<float>(ssize[0].number) : 0.0f;
			cell.ssize.y = ssize.size() > 1 ? static_cast<float>(ssize[1].number) : 0.0f;
			al.list.push_back(cell);
		}
		af->clList.push_back(al);
	}

	return af;
}

} // namespace Sprite
