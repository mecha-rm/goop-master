#include "Mesh.h"

// constructor
Mesh::Mesh(Vertex* vertices, size_t numVerts, uint32_t* indices, size_t numIndices) {
	myIndexCount = numIndices;
	myVertexCount = numVerts;
	// Create and bind our vertex array
	glCreateVertexArrays(1, &myVao);
	glBindVertexArray(myVao);
	// Create 2 buffers, 1 for vertices and the other for indices
	glCreateBuffers(2, myBuffers); // generates a name interally so you can use direct state access.

	// Bind and buffer our vertex data
	glBindBuffer(GL_ARRAY_BUFFER, myBuffers[0]);
	// upload the number of vertices. We're sticking to static draw for now, which is
	glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vertex), vertices, GL_STATIC_DRAW);

	// Bind and buffer our index data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myBuffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(uint32_t), indices, GL_STATIC_DRAW);

	// Get a null vertex to get member offsets from
	Vertex* vert = nullptr;

	// Enable vertex attribute 1 (position)
	glEnableVertexAttribArray(0);

	// Our first attribute is 3 floats, the distance between
	// them is the size of our vertex, and they will map to the position in our vertices
	// index, three floats, not normalized, size of data
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), &(vert->Position));

	// Enable vertex attribute 2 (colour)
	glEnableVertexAttribArray(1);

	// Our second attribute is 4 floats, the distance between
	// them is the size of our vertex, and they will map to the color in our vertices
	glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(Vertex), &(vert->Color));

	// Enable vertex attribute 3 (vertex normal)
	glEnableVertexAttribArray(2);

	// Our third attribute is 3 floats, the distance between
	// them is the size of our vertex, and they will map to the color in our vertices
	glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(Vertex), &(vert->Normal));

	// UV support
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, false, sizeof(Vertex), &(vert->UV));

	// Unbind our VAO (just ot be safe and make sure we don't modify it by accident)
	glBindVertexArray(0);
}

// destructor
Mesh::~Mesh() 
{
	// Clean up our buffers
	glDeleteBuffers(2, myBuffers);

	// Clean up our VAO
	glDeleteVertexArrays(1, &myVao);
}

// drawing function for meshes
void Mesh::Draw() {
	// Bind the mesh
	glBindVertexArray(myVao);

	// if we have indices, we enter this statement.
	if (myIndexCount > 0)
	{
		// Draw all of our vertices as triangles, our indexes are unsigned ints (uint32_t)
		glDrawElements(GL_TRIANGLES, myIndexCount, GL_UNSIGNED_INT, nullptr);
	}
	// if we don't have indices (i.e. we just have vertices), we enter this statement.
	else
	{
		// draws all our vertices as triangles, without indexes
		glDrawArrays(GL_TRIANGLES, 0, myVertexCount);
	}
}

// returns 'true' if the mesh is in wireframe mode. False otherwise.
bool Mesh::IsWireframe() { return wireframe; }

// if 'true' is passed, wireframe is enabled. If false is passed, wireframe is disabled.
void Mesh::SetWireframe(bool wf) { wireframe = wf; }

// if 'true' is passed, wireframe is enabled. If false is passed, wireframe is disabled.
void Mesh::SetWireframe() { wireframe = !wireframe; }

// enables wireframe mode.
void Mesh::enableWireframe() { this->wireframe = true; }

// disables wireframe mode.
void Mesh::DisableWireframe() { this->wireframe = false; }

// returns 'true' if the mesh is visible, false otherwise.
bool Mesh::IsVisible() { return visible; }

// toggle's visibility
void Mesh::SetVisible() { visible = !visible; }

// sets whether the object is visible or not.
void Mesh::SetVisible(bool visible) { this->visible = visible; }
