#pragma once
#include <GLM/glm.hpp>
#include <unordered_map>
#include <memory>
#include <functional>
#include "Shader.h"
#include "Texture2D.h"
#include "TextureSampler.h"
#include "TextureCube.h"

/*
Represents settings for a shader
*/
class Material {
public:
	typedef std::shared_ptr<Material> Sptr;

	std::function<void(Sptr)> PreFrame;
	bool IsBlendingEnabled;
	bool IsCullingEnabled;
	
	Material(const Shader::Sptr& shader) { myShader = shader; IsBlendingEnabled = false; IsCullingEnabled = true; }
	virtual ~Material() = default;
	
	const Shader::Sptr& GetShader() const { return myShader; }
	virtual void Apply();

	Sptr Clone();
	
	void Set(const std::string& name, const glm::mat4& value) { myMat4s[name] = value; }
	void Set(const std::string& name, const glm::vec4& value) { myVec4s[name] = value; }
	void Set(const std::string& name, const glm::vec3& value) { myVec3s[name] = value; }
	void Set(const std::string& name, const glm::vec2& value) { myVec2s[name] = value; }
	void Set(const std::string& name, int value) { myInts[name] = value; }
	void Set(const std::string& name, const float& value) { myFloats[name] = value; }

	// New in tutorial 06
	void Set(const std::string& name, const ITexture::Sptr& value, const TextureSampler::Sptr& sampler = nullptr) { myTextures[name] = { value, sampler }; }

	void DrawEditor(const std::vector<TextureSampler::Sptr>& samplerOptions, const std::vector<Texture2D::Sptr>& textureOptions);
	
protected:
	struct TextureInfo {
		ITexture::Sptr       Texture;
		TextureSampler::Sptr Sampler;
	};
	Shader::Sptr myShader;
	std::unordered_map<std::string, glm::mat4> myMat4s;
	std::unordered_map<std::string, glm::vec4> myVec4s;
	std::unordered_map<std::string, glm::vec3> myVec3s;
	std::unordered_map<std::string, glm::vec2> myVec2s;
	std::unordered_map<std::string, int> myInts;
	std::unordered_map<std::string, float> myFloats;

	// New in tutorial 06
	std::unordered_map<std::string, TextureInfo> myTextures;
};