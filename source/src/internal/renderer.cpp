#include "internal/renderer.h"


RenderMode::RenderMode()
  : onAir(false), isKaicho(false), useEye(false), useMustache(false)
{
}

Renderer::Renderer() 
  : width(0), height(0), frame_rate(0) {
}
Renderer::~Renderer() {
}

int Renderer::GetWidth() {
  return width;
}
int Renderer::GetHeight() {
  return height;
}

void Renderer::SetFrameRate(int fr) {
  frame_rate = fr;
}

std::unique_ptr<Renderer> RendererFactory::Create() {
	std::unique_ptr<Renderer> unique_ptr = std::unique_ptr<Renderer>();
	return unique_ptr;
}

