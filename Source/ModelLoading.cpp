#include "ModelLoading.h"

#include <stdexcept>

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

using namespace std::string_literals;

namespace
{
	Mesh ProcessMesh(const aiMesh* mesh)
	{
		using namespace DirectX;

		Mesh ret;

		// Index buffer
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
			{
				ret.indices.push_back(mesh->mFaces[i].mIndices[j]);
			}
		}

		// Vertex buffer
		assert(mesh->HasPositions());
		assert(mesh->HasNormals());
		if (mesh->HasTextureCoords(0))
		{
			assert(mesh->HasTangentsAndBitangents());
			std::vector<VertexP3N3T3U2> vertices;
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				VertexP3N3T3U2 v{
					XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z),
					XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z),
					XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z),
					XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
				};
				vertices.push_back(v);
			}
			ret.type = VertexType::eP3N3T3U2;
			ret.vertices.resize(vertices.size() * sizeof(vertices[0]));
			memcpy(ret.vertices.data(), vertices.data(), vertices.size() * sizeof(vertices[0]));
		}
		else
		{
			std::vector<VertexP3N3> vertices;
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				VertexP3N3 v{
					XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z),
					XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z),
				};
				vertices.push_back(v);
			}
			ret.type = VertexType::eP3N3;
			ret.vertices.resize(vertices.size() * sizeof(vertices[0]));
			memcpy(ret.vertices.data(), vertices.data(), vertices.size() * sizeof(vertices[0]));
		}
		return ret;
	}

	void ProcessNode(const aiScene* scene, const aiNode* node, std::vector<Mesh>& meshes)
	{
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			const aiNode* child = node->mChildren[i];
			ProcessNode(scene, child, meshes);
		}

		if (node->mNumMeshes > 0)
		{
			unsigned int meshIdx = node->mMeshes[0];
			const aiMesh* mesh = scene->mMeshes[meshIdx];
			meshes.push_back(ProcessMesh(mesh));
		}
	}
}

std::vector<Mesh> LoadModel(const std::string& path)
{
	std::vector<Mesh> ret;

	// Load model
	Assimp::Importer importer;
	constexpr auto flags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_TransformUVCoords
		| aiProcess_FixInfacingNormals | aiProcess_FlipUVs | aiProcess_FlipWindingOrder | aiProcess_PreTransformVertices;
	const aiScene* scene = importer.ReadFile(path, flags);
	if (!scene)
	{
		throw std::runtime_error("Import failed with error: "s + importer.GetErrorString());
	}

	const aiNode* root = scene->mRootNode;
	ProcessNode(scene, root, ret);
	return ret;
}
