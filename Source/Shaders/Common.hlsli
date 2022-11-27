struct GlobalConstants
{
	float3 eyePosition;
	float4x4 viewProj;
};

[[vk::binding(0, 0)]] ConstantBuffer<GlobalConstants> g_constants;

[[vk::binding(1, 0)]] ByteAddressBuffer g_vertices;
[[vk::binding(2, 0)]] ByteAddressBuffer g_materials;
[[vk::binding(3, 0)]] ByteAddressBuffer g_transforms;

[[vk::binding(4, 0)]] SamplerState g_samplers[24];

[[vk::binding(5, 0)]] Texture2D<float4> g_texturesFloat4[];
[[vk::binding(5, 0)]] Texture2D<float3> g_texturesFloat3[];
[[vk::binding(5, 0)]] Texture2D<float2> g_texturesFloat2[];
[[vk::binding(5, 0)]] Texture2D<float> g_texturesFloat[];