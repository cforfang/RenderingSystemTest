#include "PostProcess.h"

bool PostProcess_SSAO::Init(Graphics::RenderingSystem& ri, Graphics::BufferHandle fullscreenQuad)
{
	m_renderingSystem = &ri;

	m_fullscreenquadBuffer = fullscreenQuad;

	m_ssaoProgram = m_renderingSystem->CreateShaderProgram(
		Graphics::ShaderInfo::VSFS("Shaders/SSAO.vs", "Shaders/SSAO.fs", "Shaders/")
	);

	return true;
}

void PostProcess_SSAO::Run(const Graphics::RenderTargetHandle& outRT)
{
	// See assumptions about already bound tex-units in header

	m_renderingSystem->BindRenderTarget(outRT);
	m_renderingSystem->ClearScreen(Graphics::ClearState::AllBuffers());

	m_renderingSystem->UseShaderProgram(m_ssaoProgram);
	m_renderingSystem->Draw(m_fullscreenquadBuffer, Graphics::BufferHandle::Invalid(), 6);
}