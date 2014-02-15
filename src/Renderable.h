#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "UBOsAndMesh.h"
#include "Material.h"
#include "MeshUtils.h"

class Renderable
{
public:
	// Currently up to programmer to make sure the buffers referred to by the mesh
	// and material aren't deleted before the renderable. 
	// (Could use handles with generation/counter number to detect this.)

	Renderable(const Mesh& mesh, const Material& material)
		: m_position(glm::vec3(0, 0, 0)), m_scale(1.0f), m_material(material), m_mesh(mesh)
	{

	}

	void SetPosition(const glm::vec3& position)
	{
		m_position = position;
	}

	const glm::vec3& GetPosition() const
	{
		return m_position;
	}

	void SetScale(float scale)
	{
		m_scale = scale; // Only uniform scaling
	}

	float GetScale() const
	{
		return m_scale;
	}

	float GetRadius() const
	{
		return m_mesh.radius * m_scale;
	}

	glm::mat4 GetModelMatrix() const
	{
		return glm::translate(glm::mat4(), GetPosition()) * glm::scale(glm::mat4(), glm::vec3(m_scale));
	}

	const Material& GetMaterial() const
	{
		return m_material;
	}

	const Mesh& GetMesh() const
	{
		return m_mesh;
	}

private:
	glm::vec3 m_position;
	float     m_scale;
	Material  m_material;
	Mesh      m_mesh;
};