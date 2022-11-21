#include "Common.hlsli"

struct Vertex
{
    float3 position;
    float3 normal;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 color : Color;
};

VSOutput main(uint id : SV_VertexID)
{
    VSOutput output;
    Vertex vertex = g_vertices.Load<Vertex>(id * sizeof(Vertex));
    output.position = mul(float4(vertex.position, 1.0), g_constants.viewProj);
    output.color = abs(vertex.normal);
    return output;
}