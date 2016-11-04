Texture2D t0 : register(t0);
SamplerState s0 : register(s0);

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
	return t0.Sample(s0, input.texcoord) * input.color;
}