/**
* VertexShader.hlsl
*/

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

PSInput main(float3 pos : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD)
{
	PSInput input;
	input.position = float4(pos, 1.0f);
	input.color = color;
	input.texcoord = texcoord;
	return input;
}