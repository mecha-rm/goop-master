#pragma once

#include <glad/glad.h>
#include <GLM/glm.hpp> // For vec3 and vec4
#include <cstdint> // Needed for uint32_t
#include <memory> // Needed for smart pointers

#include "Utils.h"
#include "GraphicsResource.h"

struct Vertex {
	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec3 Normal;

	// New in tutorial 06
	glm::vec2 UV;
};

class Mesh : public GraphicsResource<GL_BUFFER> {
public:
	GraphicsClass(Mesh);
	
	// Creates a new mesh from the given vertices and indices
	Mesh(Vertex* vertices, GLsizei numVerts, uint32_t* indices, GLsizei numIndices);
	~Mesh();

	void SetDebugName(const std::string& name) override;

	// Draws this mesh
	void Draw();

private:
	// 0 is vertices, 1 is indices
	GLuint myBuffers[2];
	// The number of vertices and indices in this mesh
	GLsizei myVertexCount, myIndexCount;
};
