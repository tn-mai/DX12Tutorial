Texture2D t0 : register(t0);
SamplerState s0 : register(s0);

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

cbuffer Constant : register(b0)
{
	float scrollOffset;
}

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

	const float4 colorList[] = {
		float4(0.1f, 0.2f, 0.5f, 1.0f),
		float4(0.7f, 0.9f, 1.0f, 1.0f),
		float4(0.7f, 0.5f, 0.1f, 1.0f),
		float4(0.3f, 0.65f, 0.1f, 1.0f),
		float4(0.95f, 1.0f, 1.0f, 1.0f)
	};

	const float offsetList[] = { 0.0f, 0.4f, 0.43f, 0.65f, 1.0f };

	float value = 0.0f;
	float scale = 0.5f;
	float freq = 3.0f;
	float2 pos = input.texcoord + float2(0, scrollOffset);
	for (int i = 0; i <= 4; ++i) {
		value += Noise(pos * freq) * scale;;
		scale *= 0.5f;
		freq *= 2.0f;
	}
	float valueLeft = 0.0f;
	scale = 0.5f;
	freq = 3.0f;
	pos.x -= 1.0f / 800.0f;
	for (i = 0; i <= 4; ++i) {
		valueLeft += Noise(pos * freq) * scale;;
		scale *= 0.5f;
		freq *= 2.0f;
	}

	float4 color = colorList[4];
	for (i = 1; i < 5; ++i) {
		if (value <= offsetList[i]) {
			float range = offsetList[i] - offsetList[i - 1];
			color = lerp(colorList[i - 1], colorList[i], (value - offsetList[i - 1]) / range);
			if (value > offsetList[1] && value < valueLeft) {
				color *= 1.0f + (value - valueLeft) * 50.0f;
			}
			break;
		}
	}
	return color;
	//return value + t0.Sample(s0, input.texcoord) * input.color;
}