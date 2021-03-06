/**
* VertexShader.hlsl
*/

cbuffer RootConstants
{
	float4x4 matViewProjection;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

PSInput main(float3 pos : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD)
{
	PSInput input;
	input.position = mul(float4(pos, 1.0f), matViewProjection);
	input.color = color;
	input.texcoord = texcoord;
	return input;
}