/**
* @file Main.cpp
*/
#include <Windows.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

const wchar_t windowClassName[] = L"DX12TutorialApp";
const wchar_t windowTitle[] = L"DX12Tutorial";
const int clientWidth = 800;
const int clientHeight = 600;
HWND hwnd = nullptr;

HRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

static const int frameBufferCount = 2;

ComPtr<ID3D12Device> device;
ComPtr<IDXGISwapChain3> swapChain;
ComPtr<ID3D12CommandQueue> commandQueue;
ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
ComPtr<ID3D12Resource> renderTargetList[frameBufferCount];
ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
ComPtr<ID3D12Resource> depthStencilBuffer;
ComPtr<ID3D12CommandAllocator> commandAllocator[frameBufferCount];
ComPtr<ID3D12GraphicsCommandList> commandList;
ComPtr<ID3D12Fence> fence;
HANDLE fenceEvent;
UINT64 fenceValue[frameBufferCount];
int currentFrameIndex;
int rtvDescriptorSize;
bool warp;

ComPtr<ID3D12RootSignature> rootSignature;
ComPtr<ID3D12PipelineState> pso;

ComPtr<ID3D12Resource> vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

ComPtr<ID3D12Resource> indexBuffer;
D3D12_INDEX_BUFFER_VIEW indexBufferView;

D3D12_VIEWPORT viewport;
D3D12_RECT scissorRect;
XMFLOAT4X4 matViewProjection;

bool InitializeD3D();
void FinalizeD3D();
bool Render();
bool WaitForPreviousFrame();
bool WaitForGpu();

bool LoadShader(const wchar_t* filename, const char* target, ID3DBlob** blob);
bool CreatePSO();
bool CreateVertexBuffer();
bool CreateIndexBuffer();
void DrawTriangle();
void DrawRectangle();

/// ���_�f�[�^�^.
struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};

/// ���_�f�[�^�^�̃��C�A�E�g.
const D3D12_INPUT_ELEMENT_DESC vertexLayout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

/**
* ���_�f�[�^�z��.
*/
static const Vertex vertices[] = {
	{ XMFLOAT3( 0.0f,  0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3( 0.5f, -0.5f, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

	{ XMFLOAT3(-0.3f,  0.4f, 0.4f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3( 0.2f,  0.4f, 0.4f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3( 0.2f, -0.1f, 0.4f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(-0.3f, -0.1f, 0.4f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

	{ XMFLOAT3(-0.2f,  0.1f, 0.6f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	{ XMFLOAT3( 0.3f,  0.1f, 0.6f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	{ XMFLOAT3( 0.3f, -0.4f, 0.6f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(-0.2f, -0.4f, 0.6f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
};

/**
* �C���f�b�N�X�f�[�^�z��.
*/
static const uint32_t indices[] = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4,
};

/// �O�p�`�̕`��Ŏg�p���钸�_��.
const UINT triangleVertexCount = 3;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wc.lpszClassName = windowClassName;
	RegisterClassEx(&wc);

	RECT windowRect = { 0, 0, clientWidth, clientHeight };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	hwnd = CreateWindowEx(
		0,
		windowClassName,
		windowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	InitializeD3D();
	for (;;) {
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			break;
		}
		if (!Render()) {
			break;
		}
	}
	FinalizeD3D();

	return 0;
}

HRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE) {
			DestroyWindow(hwnd);
			return 0;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

bool InitializeD3D()
{
#ifndef NDEBUG
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
			debugController->EnableDebugLayer();
		}
	}
#endif

	ComPtr<IDXGIFactory4> dxgiFactory;
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)))) {
		return false;
	}

	// �@�\���x��11�𖞂����n�[�h�E�F�A�A�_�v�^���������A���̃f�o�C�X�C���^�[�t�F�C�X���擾����.
	ComPtr<IDXGIAdapter1> dxgiAdapter; // �f�o�C�X�����擾���邽�߂̃C���^�[�t�F�C�X
	int adapterIndex = 0; // �񋓂���f�o�C�X�̃C���f�b�N�X
	bool adapterFound = false; // �ړI�̃f�o�C�X�������������ǂ���
	while (dxgiFactory->EnumAdapters1(adapterIndex, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC1 desc;
		dxgiAdapter->GetDesc1(&desc);
		if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
			if (SUCCEEDED(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))) {
				adapterFound = true;
				break;
			}
		}
		++adapterIndex;
	}
	if (!adapterFound) {
		// �@�\���x��11�𖞂����n�[�h�E�F�A��������Ȃ��ꍇ�AWARP�f�o�C�X�̍쐬�����݂�.
		if (FAILED(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter)))) {
			return false;
		}
		warp = true;
	}
	if (FAILED(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) {
		return false;
	}

	// �R�}���h�L���[���쐬.
	const D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	if (FAILED(device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue)))) {
		return false;
	}

	// �X���b�v�`�F�[�����쐬.
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = clientWidth;
	scDesc.Height = clientHeight;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.SampleDesc.Count = 1;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = frameBufferCount;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	ComPtr<IDXGISwapChain1> tmpSwapChain;
	if (FAILED(dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &scDesc, nullptr, nullptr, &tmpSwapChain))) {
		return false;
	}
	tmpSwapChain.As(&swapChain);
	currentFrameIndex = swapChain->GetCurrentBackBufferIndex();

	// RTV�p�̃f�X�N���v�^�q�[�v�y�уf�X�N���v�^���쐬.
	D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
	rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDesc.NumDescriptors = frameBufferCount;
	rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&rtvDescriptorHeap)))) {
		return false;
	}
	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < frameBufferCount; ++i) {
		if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargetList[i])))) {
			return false;
		}
		device->CreateRenderTargetView(renderTargetList[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += rtvDescriptorSize;
	}

	// �f�v�X�X�e���V���o�b�t�@���쐬.
	D3D12_CLEAR_VALUE dsClearValue = {};
	dsClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	dsClearValue.DepthStencil.Depth = 1.0f;
	dsClearValue.DepthStencil.Stencil = 0;
	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, clientWidth, clientHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&dsClearValue,
		IID_PPV_ARGS(&depthStencilBuffer)
	))) {
		return false;
	}

	// DSV�p�̃f�X�N���v�^�q�[�v�y�уf�X�N���v�^���쐬.
	D3D12_DESCRIPTOR_HEAP_DESC dsvDesc = {};
	dsvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDesc.NumDescriptors = 1;
	dsvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(device->CreateDescriptorHeap(&dsvDesc, IID_PPV_ARGS(&dsvDescriptorHeap)))) {
		return false;
	}
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
	device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// �R�}���h�A���P�[�^���쐬.
	for (int i = 0; i < frameBufferCount; ++i) {
		if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i])))) {
			return false;
		}
	}

	// �R�}���h���X�g���쐬.
	if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[currentFrameIndex].Get(), nullptr, IID_PPV_ARGS(&commandList)))) {
		return false;
	}
	if (FAILED(commandList->Close())) {
		return false;
	}

	// �t�F���X�ƃt�F���X�C�x���g���쐬.
	if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) {
		return false;
	}
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (!fenceEvent) {
		return false;
	}
	for (int i = 0; i < frameBufferCount; ++i) {
		fenceValue[i] = 0;
	}

	if (!CreatePSO()) {
		return false;
	}
	if (!CreateVertexBuffer()) {
		return false;
	}
	if (!CreateIndexBuffer()) {
		return false;
	}

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = clientWidth;
	viewport.Height = clientHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = clientWidth;
	scissorRect.bottom = clientHeight;

	const XMMATRIX scaling = XMMatrixScaling(100.0f, 100.0f, 1.0f);
	const XMMATRIX ortho = XMMatrixOrthographicLH(static_cast<float>(clientWidth), static_cast<float>(clientHeight), 1.0f, 1000.0f);
	XMStoreFloat4x4(&matViewProjection, scaling * ortho);

	return true;
}

void FinalizeD3D()
{
	WaitForGpu();
	CloseHandle(fenceEvent);
}

bool Render()
{
	if (!WaitForPreviousFrame()) {
		return false;
	}

	if (FAILED(commandAllocator[currentFrameIndex]->Reset())) {
		return false;
	}
	if (FAILED(commandList->Reset(commandAllocator[currentFrameIndex].Get(), nullptr))) {
		return false;
	}

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargetList[currentFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += currentFrameIndex * rtvDescriptorSize;
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	DrawTriangle();
	DrawRectangle();

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargetList[currentFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	if (FAILED(commandList->Close())) {
		return false;
	}

	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	if (FAILED(swapChain->Present(1, 0))) {
		return false;
	}

	if (FAILED(commandQueue->Signal(fence.Get(), fenceValue[currentFrameIndex]))) {
		return false;
	}
	return true;
}

bool WaitForPreviousFrame()
{
	if (fence->GetCompletedValue() < fenceValue[currentFrameIndex]) {
		if (FAILED(fence->SetEventOnCompletion(fenceValue[currentFrameIndex], fenceEvent))) {
			return false;
		}
		WaitForSingleObject(fenceEvent, INFINITE);
	}
	++fenceValue[currentFrameIndex];
	currentFrameIndex = swapChain->GetCurrentBackBufferIndex();
	return true;
}

bool WaitForGpu()
{
	++fenceValue[currentFrameIndex];
	if (FAILED(commandQueue->Signal(fence.Get(), fenceValue[currentFrameIndex]))) {
		return false;
	}
	if (FAILED(fence->SetEventOnCompletion(fenceValue[currentFrameIndex], fenceEvent))) {
		return false;
	}
	WaitForSingleObject(fenceEvent, INFINITE);
	return true;
}

/**
* �V�F�[�_��ǂݍ���.
*
* @param filename �V�F�[�_�t�@�C����.
* @param target   �ΏۂƂ���V�F�[�_�o�[�W����.
* @param blob     �ǂݍ��񂾃V�F�[�_���i�[����Blob�C���^�[�t�F�C�X�|�C���^�̃A�h���X.
*
* @retval true �ǂݍ��ݐ���.
* @retval false �ǂݍ��ݎ��s.
*/
bool LoadShader(const wchar_t* filename, const char* target, ID3DBlob** blob)
{
	ComPtr<ID3DBlob> errorBuffer;
	if (FAILED(D3DCompileFromFile(filename, nullptr, nullptr, "main", target, D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, blob, &errorBuffer))) {
		if (errorBuffer) {
			OutputDebugStringA(static_cast<char*>(errorBuffer->GetBufferPointer()));
		}
		return false;
	}
	return true;
}

/**
* ���[�g�V�O�l�`����PSO���쐬����.
*/
bool CreatePSO()
{
	// ���_�V�F�[�_���쐬.
	ComPtr<ID3DBlob> vertexShaderBlob;
	if (!LoadShader(L"Res/VertexShader.hlsl", "vs_5_1", &vertexShaderBlob)) {
		return false;
	}
	// �s�N�Z���V�F�[�_���쐬.
	ComPtr<ID3DBlob> pixelShaderBlob;
	if (!LoadShader(L"Res/PixelShader.hlsl", "ps_5_1", &pixelShaderBlob)) {
		return false;
	}

	// ���[�g�V�O�l�`�����쐬.
	// ���[�g�p�����[�^��ShaderVisibility�͓K�؂ɐݒ肷��K�v������.
	// ���[�g�V�O�l�`�����������ݒ肳��Ă��Ȃ��ꍇ�ł��A�V�O�l�`���̍쐬�ɂ͐������邱�Ƃ�����.
	// ���������̏ꍇ�APSO�쐬���ɃG���[����������.
	{
		CD3DX12_ROOT_PARAMETER rootParameters[1];
		rootParameters[0].InitAsConstants(16, 0, 0);
		D3D12_ROOT_SIGNATURE_DESC rsDesc = {
			_countof(rootParameters),
			rootParameters,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		};
		ComPtr<ID3DBlob> signatureBlob;
		if (FAILED(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signatureBlob, nullptr))) {
			return false;
		}
		if (FAILED(device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)))) {
			return false;
		}
	}

	// �p�C�v���C���X�e�[�g�I�u�W�F�N�g(PSO)���쐬.
	// PSO�́A�����_�����O�p�C�v���C���̏�Ԃ�f�����A�ꊇ���ĕύX�ł���悤�ɓ������ꂽ.
	// PSO�ɂ���āA�����̃X�e�[�g�ɑ΂��Ă��ꂼ���ԕύX�R�}���h�𑗂炸�Ƃ��A�P��PSO��؂�ւ���R�}���h�𑗂邾���ōς�.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	psoDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.InputLayout.pInputElementDescs = vertexLayout;
	psoDesc.InputLayout.NumElements = sizeof(vertexLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc = { 1, 0 };
	if (warp) {
		psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
	}
	if (FAILED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)))) {
		return false;
	}
	return true;
}

/**
* ���_�o�b�t�@���쐬����.
*/
bool CreateVertexBuffer()
{
	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer)))) {
		return false;
	}
	vertexBuffer->SetName(L"Vertex buffer");

	void* pVertexDataBegin;
	const D3D12_RANGE readRange = { 0, 0 };
	if (FAILED(vertexBuffer->Map(0, &readRange, &pVertexDataBegin))) {
		return false;
	}
	memcpy(pVertexDataBegin, vertices, sizeof(vertices));
	vertexBuffer->Unmap(0, nullptr);

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = sizeof(vertices);

	return true;
}

/**
* �C���f�b�N�X�o�b�t�@���쐬����.
*/
bool CreateIndexBuffer()
{
	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuffer)))) {
		return false;
	}
	indexBuffer->SetName(L"Index buffer");

	void* pIndexDataBegin;
	const D3D12_RANGE readRange = { 0, 0 };
	if (FAILED(indexBuffer->Map(0, &readRange, &pIndexDataBegin))) {
		return false;
	}
	memcpy(pIndexDataBegin, indices, sizeof(indices));
	indexBuffer->Unmap(0, nullptr);

	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = sizeof(indices);

	return true;
}

/**
* �O�p�`��`�悷��.
*/
void DrawTriangle()
{
	commandList->SetPipelineState(pso.Get());
	commandList->SetGraphicsRootSignature(rootSignature.Get());
	commandList->SetGraphicsRoot32BitConstants(0, 16, &matViewProjection, 0);
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->DrawInstanced(triangleVertexCount, 1, 0, 0);
}

/**
* �l�p�`��`�悷��.
*/
void DrawRectangle()
{
	commandList->IASetIndexBuffer(&indexBufferView);
	commandList->DrawIndexedInstanced(_countof(indices), 1, 0, triangleVertexCount, 0);
}