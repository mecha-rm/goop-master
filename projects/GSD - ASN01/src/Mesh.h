#pragma once
#include <glad/glad.h>
#include <GLM/glm.hpp> // For vec3 and vec4
#include <cstdint> // Needed for uint32_t
#include <memory> // Needed for smart pointers

#include "Utils.h"

struct Vertex {
	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec3 Normal;
	glm::vec2 UV;
};

class Mesh {
public:
	GraphicsClass(Mesh);
	// Shorthand for shared_ptr
	// typedef std::shared_ptr<Mesh> Sptr;
	
	// Creates a new mesh from the given vertices and indices
	Mesh(Vertex* vertices, size_t numVerts, uint32_t* indices, size_t numIndices);
	
	~Mesh();

	// Draws this mesh
	void Draw();

	// Returns wireframe boolean to tell the caller if the mesh is to be drawn in wirframe
	bool IsWireframe();

	// if 'true' is passed, wireframe is enabled. If false is passed, wireframe is disabled.
	void SetWireframe(bool wf);

	// toggle's the wireframe mode on/off.
	void SetWireframe();

	// Enables wireframe mode on the mesh
	void enableWireframe();

	// Disables wireframe mode on the mesh
	void DisableWireframe();

	// returns 'true' if the mesh is visible, false otherwise.
	bool IsVisible();

	// toggle's the visibility of the mesh on/off.
	void SetVisible();

	// sets whether the mesh is visible or not.
	void SetVisible(bool visible);

private:
	// Our GL handle for the Vertex Array Object
	GLuint myVao;
	
	// 0 is vertices, 1 is indices
	GLuint myBuffers[2];
	
	// The number of vertices and indices in this mesh
	size_t myVertexCount, myIndexCount;

	// Whether or not the mesh should be drawn in wireframe mode
	bool wireframe = false;

	// boolan for visibility
	bool visible = true;
};