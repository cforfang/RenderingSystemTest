#pragma once

#include <string>
#include <vector>

#include "graphics/ForwardDecl.h"
#include "graphics/Handles.h"
#include "graphics/EnumsFlags.h"

class TextureLoader
{
public:
	struct TextureToLoad
	{
		Graphics::Texture2DHandle handle;
		std::string imageFile;
		Graphics::TextureType type;
	};

	void Schedule(const TextureToLoad& toLoad)
	{
		m_toLoadVector.push_back(toLoad);
	}

	void LoadOne(Graphics::RenderingSystem& rs, const std::string& dataPrefix);

private:
	std::vector<TextureToLoad> m_toLoadVector;
};