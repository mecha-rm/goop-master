#include "Texture2D.h"
#include "Logging.h"
#include <stb_image.h>
#include <filesystem>
#include <GLM/gtc/type_ptr.hpp>

Texture2D::Texture2D(const Texture2DDescription& desc) {
	myDescription = desc;
	
	myRenderhandle = 0;
	__SetupTexture();
}

Texture2D::~Texture2D() {
	glDeleteTextures(1, &myRenderhandle);
}

void Texture2D::__SetupTexture() {
	glCreateTextures(GL_TEXTURE_2D, 1, &myRenderhandle);
	glTextureStorage2D(myRenderhandle, myDescription.EnableMip ? myDescription.MipLevels : 1, (GLenum)myDescription.Format, myDescription.Width, myDescription.Height);

	glTextureParameteri(myRenderhandle, GL_TEXTURE_WRAP_S,     (GLenum)myDescription.Sampler.WrapS);
	glTextureParameteri(myRenderhandle, GL_TEXTURE_WRAP_T,     (GLenum)myDescription.Sampler.WrapT);
	glTextureParameteri(myRenderhandle, GL_TEXTURE_MIN_FILTER, (GLenum)myDescription.Sampler.MinFilter);
	glTextureParameteri(myRenderhandle, GL_TEXTURE_MAG_FILTER, (GLenum)myDescription.Sampler.MagFilter);

	glTextureParameterfv(myRenderhandle, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(myDescription.Sampler.BorderColor));

	if (myDescription.Sampler.AnisotropicEnabled)
		glTextureParameterf(myRenderhandle, GL_TEXTURE_MAX_ANISOTROPY, myDescription.Sampler.MaxAnisotropy);
}

void Texture2D::LoadData(void* data, size_t width, size_t height, PixelFormat format, PixelType type) {
	// TODO: Re-create texture if our data is a different size
	
	LOG_ASSERT(width == myDescription.Width, "Width of data does not match the width of this texture!");
	LOG_ASSERT(height == myDescription.Height, "Height of data does not match the width of this texture!");
	
	glTextureSubImage2D(myRenderhandle, 0, 0, 0, myDescription.Width, myDescription.Height, (GLenum)format, (GLenum)type, data);

	if (myDescription.EnableMip)
		glGenerateTextureMipmap(myRenderhandle);
}

Texture2D::Sptr Texture2D::LoadFromFile(const std::string& fileName, bool loadAlpha) {

	int width, height, numChannels;
	void* data = stbi_load(fileName.c_str(), &width, &height, &numChannels, loadAlpha ? 4 : 3);

	if (data != nullptr && width != 0 && height != 0 && numChannels != 0) {
		Texture2DDescription desc = Texture2DDescription();
		desc.Width = width;
		desc.Height = height;
		desc.Format = loadAlpha ? InternalFormat::RGBA8 : InternalFormat::RGB8;
		
		Sptr result = std::make_shared<Texture2D>(desc);
		result->LoadData(data, width, height, loadAlpha? PixelFormat::Rgba : PixelFormat::Rgb, PixelType::UByte);
		stbi_image_free(data);
		result->SetDebugName(std::filesystem::path(fileName).filename().string());
		return result;
	} else {
		LOG_WARN("Failed to load image from \"{}\"", fileName);
		return nullptr;
	}
}

