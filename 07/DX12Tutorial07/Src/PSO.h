/**
* @file PSO.h
*/
#ifndef DX12TUTORIAL_SRC_PSO_H_
#define DX12TUTORIAL_SRC_PSO_H_
#include <d3d12.h>
#include <wrl/client.h>

/**
* ルートシグネチャとPSOをまとめた構造体.
*/
struct PSO
{
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
};

/**
* PSOの種類.
*/
enum PSOType {
	PSOType_Simple,
	PSOType_NoiseTexture,
	PSOType_Sprite,
	countof_PSOType
};

bool CreatePSOList(ID3D12Device* device, bool warp);
const PSO& GetPSO(PSOType);

#endif // DX12TUTORIAL_SRC_PSO_H_