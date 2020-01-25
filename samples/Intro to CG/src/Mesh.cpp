#include "Mesh.h"

Mesh::Mesh(Vertex* vertices, GLsizei numVerts, uint32_t* indices, GLsizei numIndices) {
	myIndexCount = numIndices;
	myVertexCount = numVerts;

	// Create and bind our vertex array
	glCreateVertexArrays(1, &myRenderhandle);
	glBindVertexArray(myRenderhandle);

	// Create 2 buffers, 1 for vertices and the other for indices
	glCreateBuffers(2, myBuffers);

	// Bind and buffer our vertex data
	glBindBuffer(GL_ARRAY_BUFFER, myBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vertex), vertices, GL_STATIC_DRAW);

	// Bind and buffer our index data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myBuffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(uint32_t), indices, GL_STATIC_DRAW);

	#pragma warning(push)
	#pragma warning(disable: 6011)
	
	// Get a null vertex to get member offsets from
	Vertex* vert = nullptr;
	
	// Enable vertex attribute 0
	glEnableVertexAttribArray(0);
	// Our first attribute is 3 floats, the distance between 
	// them is the size of our vertex, and they will map to the position in our vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), &(vert->Position)); 

	// Enable vertex attribute 1
	glEnableVertexAttribArray(1);
	// Our second attribute is 4 floats, the distance between 
	// them is the size of our vertex, and they will map to the color in our vertices
	glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(Vertex), &(vert->Color));


	// Enable vertex attribute 2
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(Vertex), &(vert->Normal));

	// New in tutorial 06
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, false, sizeof(Vertex), &(vert->UV));

	#pragma warning(pop)
	
	// Unbind our VAO
	glBindVertexArray(0);
}

Mesh::~Mesh() {
	// Clean up our buffers
	glDeleteBuffers(2, myBuffers);
	// Clean up our VAO
	glDeleteVertexArrays(1, &myRenderhandle);
}

void Mesh::SetDebugName(const std::string& name) {
	glObjectLabel(GL_VERTEX_ARRAY, myRenderhandle, -1, name.c_str());
	glObjectLabel(GL_BUFFER, myBuffers[0], -1, (name + " | VBO").c_str());
	glObjectLabel(GL_BUFFER, myBuffers[1], -1, (name + " | IBO").c_str());
}

void Mesh::Draw() {
	// Bind the mesh
	glBindVertexArray(myRenderhandle);
	if (myIndexCount > 0) {
		// Draw all of our vertices as triangles, our indexes are unsigned ints (uint32_t)
		glDrawElements(GL_TRIANGLES, myIndexCount, GL_UNSIGNED_INT, nullptr);
	} else {
		// Draw all of our vertices as triangles, our indexes are unsigned ints (uint32_t)
		glDrawArrays(GL_TRIANGLES, 0, myVertexCount);
	}
}
