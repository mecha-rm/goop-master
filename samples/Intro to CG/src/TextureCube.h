#pragma once
#include <glad/glad.h>
#include <string>
#include <EnumToString.h>
#include "Texture2D.h"
#include "Utils.h"

/*
0 	GL_TEXTURE_CUBE_MAP_POSITIVE_X
1 	GL_TEXTURE_CUBE_MAP_NEGATIVE_X
2 	GL_TEXTURE_CUBE_MAP_POSITIVE_Y
3 	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
4 	GL_TEXTURE_CUBE_MAP_POSITIVE_Z
5 	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
 */

ENUM(CubeMapFace, GLint,
    PosX = 0,
	NegX = 1,
    PosY = 2,
	NegY = 3,
	PosZ = 4,
	NegZ = 5
);

struct TextureCubeDesc {
	uint32_t       Size        = 0;
	InternalFormat Format      = InternalFormat::RGBA8;
};


class TextureCube : public ITexture {
public:
	GraphicsClass(TextureCube);

	TextureCube(const TextureCubeDesc& desc);
	virtual ~TextureCube();

	void LoadData(uint32_t width, uint32_t height, CubeMapFace face, PixelFormat format, PixelType type, void* data);

	static Sptr LoadFromFiles(const std::string faceFiles[6], bool flipVertically = true);
	
protected:
	TextureCubeDesc myDesc;

	void __InitTexture();
};