#include "Common.hlsli"

struct Vertex
{
    float3 position;
    float3 normal;
    float3 tangent;
    float2 texcoord;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 texcoord : Texcoord;
};

VSOutput main(uint id : SV_VertexID)
{
    VSOutput output;
    Vertex vertex = g_vertices.Load<Vertex>(id * sizeof(Vertex));
    output.position = mul(float4(vertex.position, 1.0), g_constants.viewProj);
    output.texcoord = vertex.texcoord;
    return output;
}