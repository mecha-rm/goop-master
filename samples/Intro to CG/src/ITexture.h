#pragma once
#include <memory>
#include "GraphicsResource.h"

class ITexture : public GraphicsResource<GL_TEXTURE> {
public:
	typedef std::shared_ptr<ITexture> Sptr;

	virtual void Bind(int slot) { glBindTextureUnit(slot, myRenderhandle); }
	static void Unbind(int slot) { glBindTextureUnit(slot, 0); }
	
protected:
	ITexture() = default;
	virtual ~ITexture() = default;
};
