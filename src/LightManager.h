#pragma once

#include "glm/glm.hpp"
#include "Constants.h"
#include "graphics/RenderingSystem.h"

template<uint32_t MAX_LIGHTS>
class LightManager
{
public:
	LightManager()
	{
		memset(&m_lights, 0, sizeof(m_lights));
		m_lights.num = MAX_LIGHTS;
	}

	void Init(Graphics::RenderingSystem& renderingSystem)
	{
		m_pRenderingSystem = &renderingSystem;

		m_lightsBuffer = renderingSystem.CreateBuffer();
		renderingSystem.UpdateBuffer(m_lightsBuffer, &m_lights, sizeof(m_lights), Graphics::BufferType::STATIC);
		renderingSystem.BindUniformBuffer(Constants::LIGHTS_UBO_BINDING_INDEX, m_lightsBuffer);
	}

	void Update(double time, glm::vec3 cameraPosition)
	{
		// Animate (Sponza spesific)
		for (uint32_t i = 0; i < m_lights.num-1; ++i)
		{
			Light& l = m_lights.lights[i];

			float a = i / float(m_lights.num-1);
			float aa = a * 3.14f * 2.0f + (float) time;

			// Move in ellipse on ground
			l.position = glm::vec4(sin(aa) * 12, 0.25, cos(aa) * 0.5 - 0.25, 0.0);

			// Funky colors
			l.diffuse = glm::vec4(sin(aa + time), sin(aa + 2 + time), sin(aa + 4 + time), 1) * glm::vec4(0.5, 0.5, 0.5, 1.0) + glm::vec4(0.5, 0.5, 0.5, 0.0);
			l.specular = l.diffuse;

			l.constAttLinAttQuadrAttRange = glm::vec4(1.0f, 0.0f, 15.0f, 20.0f);
		}

		// Light @ camera position
		Light& cameraLight = m_lights.lights[m_lights.num - 1];
		cameraLight.position = glm::vec4(cameraPosition, 1.0);
		cameraLight.diffuse = glm::vec4(1.0f);
		cameraLight.specular = glm::vec4(1.0f);
		cameraLight.constAttLinAttQuadrAttRange = glm::vec4(1, 0, 0.25, 50);

		// Upload
		m_pRenderingSystem->UpdateBuffer(m_lightsBuffer, &m_lights, sizeof(m_lights), Graphics::BufferType::STATIC);
	}

	int GetActiveLights()
	{
		return m_lights.num;
	}

	glm::vec4 GetLightPosition(int index)
	{
		return m_lights.lights[index].position;
	}

private:
	struct Light
	{
		glm::vec4 position;
		glm::vec4 diffuse;
		glm::vec4 specular;
		glm::vec4 constAttLinAttQuadrAttRange;
	};

	struct LightsUBO
	{
		glm::uint num; glm::uint pad[3];
		Light lights[MAX_LIGHTS];
	};

	LightsUBO m_lights;
	Graphics::BufferHandle m_lightsBuffer;

	Graphics::RenderingSystem* m_pRenderingSystem;
	
};