#pragma once
#include <glad/glad.h>
#include <string>

template<GLuint identifier>
class GraphicsResource {
public:
	virtual inline void SetDebugName(const std::string& name) { myDebugName = name;  glObjectLabel(identifier, myRenderhandle, -1, name.c_str()); }
	const std::string& GetDebugName() const { return myDebugName; }
	
protected:
	GraphicsResource() : myRenderhandle(0), myDebugName(std::string()) {};
	virtual ~GraphicsResource() = default;
	
	GLuint myRenderhandle;
	std::string myDebugName;
};