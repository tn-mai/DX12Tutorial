/**
* @file PSO.h
*/
#ifndef DX12TUTORIAL_SRC_PSO_H_
#define DX12TUTORIAL_SRC_PSO_H_
#include <d3d12.h>
#include <wrl/client.h>

/**
* ���[�g�V�O�l�`����PSO���܂Ƃ߂��\����.
*/
struct PSO
{
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
};

/**
* PSO�̎��.
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