/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#include "renderer.h"
#include <iostream>

class RendererOpenGLImpl : public Renderer::IBackend
{
public:
	void Init();
	void ProcessCommands(const Renderer::CommandQueue &queue) override;
	void Shutdown();
	Renderer::BACKEND GetType();
};

Renderer::IBackend* CreateOpenGLRenderer()
{
	std::cerr << "Not yet implemented" << std::endl;
	return nullptr;// new RendererOpenGLImpl();
}

void RendererOpenGLImpl::Init()
{
}

void RendererOpenGLImpl::ProcessCommands(const Renderer::CommandQueue &queue)
{
}

void RendererOpenGLImpl::Shutdown()
{
}

Renderer::BACKEND RendererOpenGLImpl::GetType()
{
	return Renderer::BACKEND::OpenGL;
}
