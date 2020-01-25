#pragma once
#include "entt.hpp"
#include "TextureCube.h"
#include "Shader.h"
#include "Mesh.h"

class Scene {
public:
	Scene() = default;
	virtual ~Scene() = default;
	
	virtual void OnOpen() {};
	virtual void OnClose() {};
	
	entt::registry& Registry() { return myRegistry; }
	
	const std::string& GetName() const { return myName; }
	void SetName(const std::string& name) { myName = name; }

	// New in tutorial 09
	TextureCube::Sptr Skybox;
	Shader::Sptr      SkyboxShader;
	Mesh::Sptr        SkyboxMesh;
	
private:
	entt::registry myRegistry;
	std::string myName;
};