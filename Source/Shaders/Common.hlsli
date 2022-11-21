struct GlobalConstants
{
	float3 eyePosition;
	float4x4 viewProj;
};

[[vk::binding(0, 0)]] ConstantBuffer<GlobalConstants> g_constants;

[[vk::binding(1, 0)]] ByteAddressBuffer g_vertices;
[[vk::binding(2, 0)]] ByteAddressBuffer g_materials;
[[vk::binding(3, 0)]] ByteAddressBuffer g_transforms;