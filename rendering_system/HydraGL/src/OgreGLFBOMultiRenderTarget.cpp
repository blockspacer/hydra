/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include <OGRE/OgreLogManager.h>
#include <OGRE/OgreStringConverter.h>
#include <OGRE/OgreRoot.h>

#include "OgreGLFBOMultiRenderTarget.h"
#include "OgreGLPixelFormat.h"
#include "OgreGLHardwarePixelBuffer.h"

Ogre::GLFBOMultiRenderTarget::GLFBOMultiRenderTarget(Ogre::GLFBOManager *manager, const Ogre::String &name):
	MultiRenderTarget(name),
	fbo(manager, 0 /* TODO: multisampling on MRTs? */)
{
}
//-----------------------------------------------------------------------------
Ogre::GLFBOMultiRenderTarget::~GLFBOMultiRenderTarget()
{
}
//-----------------------------------------------------------------------------
void
Ogre::GLFBOMultiRenderTarget::bindSurfaceImpl(size_t attachment, RenderTexture *target)

{

	/// Check if the render target is in the rendertarget->FBO map
    GLFrameBufferObject *fbobj = 0;
    target->getCustomAttribute(GLRenderTexture::CustomAttributeString_FBO, &fbobj);
	assert(fbobj);
	fbo.bindSurface(attachment, fbobj->getSurface(0));



	// Initialise?

		

	// Set width and height

	mWidth = fbo.getWidth();

	mHeight = fbo.getHeight();

}
//-----------------------------------------------------------------------------
void
Ogre::GLFBOMultiRenderTarget::unbindSurfaceImpl(size_t attachment)
{
	fbo.unbindSurface(attachment);

	// Set width and height

	mWidth = fbo.getWidth();

	mHeight = fbo.getHeight();
}
//-----------------------------------------------------------------------------
void
Ogre::GLFBOMultiRenderTarget::getCustomAttribute( const String& name, void *pData )
{
	if( name == GLRenderTexture::CustomAttributeString_FBO )
    {
        *static_cast<GLFrameBufferObject **>(pData) = &fbo;
    }
}
//-----------------------------------------------------------------------------
bool
Ogre::GLFBOMultiRenderTarget::attachDepthBuffer( DepthBuffer *depthBuffer )
{
	bool result;
	if( (result = MultiRenderTarget::attachDepthBuffer( depthBuffer )) )
		fbo.attachDepthBuffer( depthBuffer );

	return result;
}
//-----------------------------------------------------------------------------
void
Ogre::GLFBOMultiRenderTarget::detachDepthBuffer()
{
	fbo.detachDepthBuffer();
	MultiRenderTarget::detachDepthBuffer();
}
//-----------------------------------------------------------------------------
void
Ogre::GLFBOMultiRenderTarget::_detachDepthBuffer()
{
	fbo.detachDepthBuffer();
	MultiRenderTarget::_detachDepthBuffer();
}
//-----------------------------------------------------------------------------
void
Ogre::GLFBOMultiRenderTarget::_beginUpdate(void)
{
	Ogre::MultiRenderTarget::_beginUpdate();

	// Bind the frame buffer
	fbo.bind();
}
//-----------------------------------------------------------------------------
void
Ogre::GLFBOMultiRenderTarget::_endUpdate(void)
{
	// Completely unbind the Framebuffer
	// This is because we want the window framebuffer to be restored
	// and Ogre does not unbind framebuffers when they are no longer used.
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	Ogre::MultiRenderTarget::_endUpdate();
}
