#pragma once

#include "Mesh.h"
#include <vector>

struct MeshData {
	std::vector<Vertex>   Vertices;
	std::vector<uint32_t> Indices;
};

class ObjLoader {
public:
	static MeshData LoadObj(const char* filename, glm::vec4 baseColor = glm::vec4(1.0f));
	static Mesh::Sptr LoadObjToMesh(const char* filename, glm::vec4 baseColor = glm::vec4(1.0f)) {
		MeshData data = LoadObj(filename, baseColor);
		auto result = std::make_shared<Mesh>(
			data.Vertices.data(), static_cast<GLsizei>(data.Vertices.size()), 
			data.Indices.data(), static_cast<GLsizei>(data.Indices.size()));
		result->SetDebugName(filename);
		return result;
	}
};
