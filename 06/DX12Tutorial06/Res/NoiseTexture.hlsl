Texture2D t0 : register(t0);
SamplerState s0 : register(s0);

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

float NoiseSeed(float2 st)
{
	return frac(sin(st.x * 12.9898f + st.y * 78.233f) * 43758.5453123f);
}

float Noise(float2 st)
{
	float2 i = floor(st);
	float2 f = frac(st);
	float2 u = f * f * (3.0f - 2.0f * f);
	const float a = NoiseSeed(i + float2(0, 0));
	const float b = NoiseSeed(i + float2(1, 0));
	const float c = NoiseSeed(i + float2(0, 1));
	const float d = NoiseSeed(i + float2(1, 1));
	return (a * (1.0f - u.x) + b * u.x) + (c - a) * u.y * (1.0f - u.x) + (d - b) * u.y * u.x;
}

float4 main(PSInput input) : SV_TARGET
{
	float value = 0.0f;
	float scale = 0.5f;
	float freq = 4.0f;
	for (float i = 0; i <= 4; ++i) {
		value += Noise(input.texcoord * freq) * scale;
		scale *= 0.5f;
		freq *= 2.0f;
	}
	return t0.Sample(s0, float2(value, input.texcoord.y * 4.0f)) * input.color;
}