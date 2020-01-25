#include "TextureSampler.h"
#include <GLM/gtc/type_ptr.hpp>

TextureSampler::TextureSampler(const SamplerDesc& desc) {
	myDesc = desc;
	
	glCreateSamplers(1, &myRenderhandle);

	glSamplerParameteri(myRenderhandle, GL_TEXTURE_WRAP_S, (GLenum)myDesc.WrapS);
	glSamplerParameteri(myRenderhandle, GL_TEXTURE_WRAP_T, (GLenum)myDesc.WrapT);
	glSamplerParameteri(myRenderhandle, GL_TEXTURE_WRAP_R, (GLenum)myDesc.WrapR);

	glSamplerParameteri(myRenderhandle, GL_TEXTURE_MIN_FILTER, (GLenum)myDesc.MinFilter);
	glSamplerParameteri(myRenderhandle, GL_TEXTURE_MAG_FILTER, (GLenum)myDesc.MagFilter);
		
	glSamplerParameterfv(myRenderhandle, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(myDesc.BorderColor));
	
	if (myDesc.AnisotropicEnabled)
		glSamplerParameterf(myRenderhandle, GL_TEXTURE_MAX_ANISOTROPY, myDesc.MaxAnisotropy);
}

TextureSampler::~TextureSampler() {
	glDeleteSamplers(1, &myRenderhandle);
}

void TextureSampler::Bind(uint32_t slot) {
	glBindSampler(slot, myRenderhandle);
}

void TextureSampler::Unbind(uint32_t slot) {
	glBindSampler(slot, 0);
}
