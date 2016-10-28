/**
* VertexShader.hlsl
*/

struct RootConstants
{
	float4x4 matViewProjection;
};
ConstantBuffer<RootConstants> rc : register(b0);

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput main(float3 pos : POSITION, float4 color : COLOR)
{
	PSInput input;
	input.position = mul(float4(pos, 1.0f), rc.matViewProjection);
	input.color = color;
	return input;
}