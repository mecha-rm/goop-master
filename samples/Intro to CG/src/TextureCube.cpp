#include "TextureCube.h"
#include "Logging.h"
#include "stb_image.h"

TextureCube::TextureCube(const TextureCubeDesc& desc) {
	myDesc = desc;
	myRenderhandle = 0;

	__InitTexture();
}

TextureCube::~TextureCube() { glDeleteTextures(1, &myRenderhandle); }

void TextureCube::LoadData(uint32_t width, uint32_t height, CubeMapFace face, PixelFormat format, PixelType type, void* data) {		
	glTextureSubImage3D(myRenderhandle, 0, 0, 0, (int)face, myDesc.Size, myDesc.Size, 1, (GLenum)format, (GLenum)type, data);
}

TextureCube::Sptr TextureCube::LoadFromFiles(const std::string faceFiles[6], bool flipVertically) {
	TextureCubeDesc desc = TextureCubeDesc();
	desc.Format = InternalFormat::RGB8;
	
    stbi_set_flip_vertically_on_load(flipVertically);
	Sptr result = nullptr;
	
	for(int ix = 0; ix < 6; ix++) {

		int width, height, numChannels;
		void* data = stbi_load(faceFiles[ix].c_str(), &width, &height, &numChannels, 3);
		
		if (desc.Size != 0 && ((width != desc.Size) | (height != desc.Size))) {
			stbi_image_free(data);
			LOG_ASSERT(false, "Image file dimensions do not match the size of this cubemap! ({})", faceFiles[ix]);
		}
		if (width != height) {
			stbi_image_free(data);
			LOG_ASSERT(false, "Image for cubemap must be square! ({})", faceFiles[ix]);
		}

		if (ix == 0) {
			desc.Size = width;
			result = std::make_shared<TextureCube>(desc);
		}
		
		if (data != nullptr && width != 0 && height != 0 && numChannels != 0) {
			result->LoadData(width, height, (CubeMapFace)ix,
				numChannels == 4 ? PixelFormat::Rgba : PixelFormat::Rgb,
				PixelType::UByte, data);
			stbi_image_free(data);
		}
		else {
			LOG_WARN("Failed to load image from \"{}\"", faceFiles[ix]);
		}
	}

	return result;
}

void TextureCube::__InitTexture() {
	GLenum format = (GLenum)myDesc.Format;
	
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &myRenderhandle);

	glTextureParameteri(myRenderhandle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(myRenderhandle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(myRenderhandle, GL_TEXTURE_MIN_FILTER, (GLenum)MinFilter::Linear);
	glTextureParameteri(myRenderhandle, GL_TEXTURE_MAG_FILTER, (GLenum)MagFilter::Linear);

		
	glTextureStorage2D(myRenderhandle, 1, format, myDesc.Size, myDesc.Size);
}
