#pragma once

#include <vector>
#include <string>

#include <DirectXMath.h>

struct VertexP3
{
	DirectX::XMFLOAT3 position;
};

struct VertexP3U2
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 texcoord;
};

struct VertexP3N3
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
};

struct VertexP3N3U2
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 texcoord;
};

struct VertexP3N3T3
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
};

struct VertexP3N3T3U2
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT2 texcoord;
};

enum class VertexType
{
	eP3,
	eP3U2,
	eP3N3,
	eP3N3U2,
	eP3N3T3,
	eP3N3T3U2
};

struct Mesh
{
	VertexType type;
	std::vector<char> vertices;
	std::vector<uint32_t> indices;
};

std::vector<Mesh> LoadModel(const std::string& filename);