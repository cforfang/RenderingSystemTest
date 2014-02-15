#include "TextureLoader.h"

#include "graphics/RenderingSystem.h"

#define STBI_HEADER_FILE_ONLY
#include "stb_image.c"

void TextureLoader::LoadOne(Graphics::RenderingSystem& rs, const std::string& dataPrefix)
{
	if (!m_toLoadVector.empty())
	{
		auto tex = m_toLoadVector.back();
		m_toLoadVector.pop_back();

		std::string fullPath = dataPrefix + tex.imageFile;
		Graphics::TextureType type = tex.type;

		int x, y, comp;
		uint8_t* textureData = stbi_load(fullPath.c_str(), &x, &y, &comp, 4);

		if (textureData)
		{
			printf("Loaded %s\n", fullPath.c_str());
			rs.UpdateTexture2D(tex.handle, textureData, x, y, type);
			stbi_image_free(textureData);
		}
		else
		{
			printf("Failed to load %s\n", fullPath.c_str());
		}
	}
}