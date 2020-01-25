#pragma once
#include <glad/glad.h>
#include <EnumToString.h>
#include <GLM/glm.hpp>
#include "Utils.h"
#include "GraphicsResource.h"

// These are our options for GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_S and GL_TEXTURE_WRAP_R
ENUM(WrapMode, GLint,
	ClampToEdge       = GL_CLAMP_TO_EDGE,
	ClampToBorder     = GL_CLAMP_TO_BORDER,
	MirroredRepeat    = GL_MIRRORED_REPEAT,
	Repeat            = GL_REPEAT, // Default
	MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE
);

// These are our available options for the GL_TEXTURE_MIN_FILTER setting
ENUM(MinFilter, GLint,
	Nearest           = GL_NEAREST,
	Linear            = GL_LINEAR,
	NearestMipNearest = GL_NEAREST_MIPMAP_NEAREST,
	LinearMipNearest  = GL_LINEAR_MIPMAP_NEAREST,
	NearestMipLinear  = GL_NEAREST_MIPMAP_LINEAR, // This is the default setting
	LinearMipLinear   = GL_LINEAR_MIPMAP_LINEAR
);

// These are our available options for the GL_TEXTURE_MAG_FILTER setting
ENUM(MagFilter, GLint,
	Nearest = GL_NEAREST,
	Linear  = GL_LINEAR  // This is the default setting
);

struct SamplerDesc {
	WrapMode WrapS                      = WrapMode::Repeat;
	WrapMode WrapT                      = WrapMode::Repeat;
	WrapMode WrapR                      = WrapMode::Repeat;
	MinFilter MinFilter                 = MinFilter::NearestMipLinear;
	MagFilter MagFilter                 = MagFilter::Linear;
	glm::vec4 BorderColor               = glm::vec4(0.0f);

	bool      AnisotropicEnabled        = true;
	float     MaxAnisotropy             = 1.0f;
};

class TextureSampler : public GraphicsResource<GL_SAMPLER> {
public:
	GraphicsClass(TextureSampler);
	
	TextureSampler(const SamplerDesc& desc = SamplerDesc());
	~TextureSampler();
	
	void Bind(uint32_t slot);
	static void Unbind(uint32_t slot);
	
	const SamplerDesc& GetDescription() const { return myDesc; }

private:
	SamplerDesc myDesc;
};