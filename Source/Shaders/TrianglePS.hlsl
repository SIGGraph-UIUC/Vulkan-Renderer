#include "Common.hlsli"

struct PSInput
{
    float4 position : SV_Position;
    float2 texcoord : Texcoord;
};

float4 main(PSInput input) : SV_Target
{
    float3 color = g_texturesFloat3[0].Sample(g_samplers[0], input.texcoord);
	return float4(color, 1.0f);
}