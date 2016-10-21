struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput main(float3 pos : POSITION, float4 color : COLOR)
{
	PSInput input;
	input.position = float4(pos, 1.0f);
	input.color = color;
	return input;
}