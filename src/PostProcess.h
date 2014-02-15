#pragma once

#include "graphics/RenderingSystem.h"

class PostProcess_SSAO
{
public:
	bool Init(Graphics::RenderingSystem& rs, Graphics::BufferHandle fullscreenQuad);

	// Assumes tex-unit 0 contains color, tex-unit 1 depth, and tex-unit 2 normals
	void Run(const Graphics::RenderTargetHandle& out);

private:
	Graphics::RenderingSystem* m_renderingSystem;
	Graphics::ShaderProgramHandle m_ssaoProgram;
	Graphics::BufferHandle m_fullscreenquadBuffer;
};