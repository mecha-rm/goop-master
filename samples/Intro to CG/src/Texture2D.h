#pragma once
#include <glad\glad.h>
#include <memory>
#include <string>
#include <EnumToString.h>
#include "TextureSampler.h"

#include "Utils.h"
#include "GraphicsResource.h"
#include "ITexture.h"

// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
// These are some of our more common available internal formats
ENUM(InternalFormat, GLint,
	Depth        = GL_DEPTH_COMPONENT,
	DepthStencil = GL_DEPTH_STENCIL,
	R8           = GL_R8,
	R16          = GL_R16,
	RGB8         = GL_RGB8,
	RGB16        = GL_RGB16,
	RGBA8        = GL_RGBA8,
	RGBA16       = GL_RGBA16

	// Note: There are sized internal formats but there is a LOT of them
);

// The layout of the input pixel data
ENUM(PixelFormat, GLint,
	Red          = GL_RED,
	Rg           = GL_RG,
	Rgb          = GL_RGB,
	Bgr          = GL_BGR,
	Rgba         = GL_RGBA,
	Bgra         = GL_BGRA,
	Depth        = GL_DEPTH_COMPONENT,
	DepthStencil = GL_DEPTH_STENCIL
);

// The type for each component of the pixel data
ENUM(PixelType, GLint,
	UByte  = GL_UNSIGNED_BYTE,
	Byte   = GL_BYTE,
	UShort = GL_UNSIGNED_SHORT,
	Short  = GL_SHORT,
	UInt   = GL_UNSIGNED_INT,
	Int    = GL_INT,
	Float  = GL_FLOAT
);

// Represents all the data required to set up our texture (but not actually load it's data)
// This is more or less all the GPU state that we care about
struct Texture2DDescription {
	uint32_t       Width     = 0;
	uint32_t       Height    = 0;
	InternalFormat Format    = InternalFormat::RGBA8;

	// New in tutorial 09
	bool           EnableMip = true;
	int            MipLevels = 8;
	SamplerDesc    Sampler   = SamplerDesc();
};

// Represents a 2D texture in OpenGL
class Texture2D : public ITexture
{
public:
	GraphicsClass(Texture2D);
	
	Texture2D(const Texture2DDescription& description);
	virtual ~Texture2D();

	void LoadData(void* data, size_t width, size_t height, PixelFormat format, PixelType type);
		
	static Sptr LoadFromFile(const std::string& fileName, bool loadAlpha = true);

protected:
	Texture2DDescription myDescription;

	void __SetupTexture();
};