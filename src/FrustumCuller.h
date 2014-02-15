#pragma once

#include <vector>
#include "glm/glm.hpp"

extern void ExtractFrustumPlanes(const glm::mat4& viewProj, glm::vec4 planes[6]);

template<typename T>
class FrustumCuller
{
public:
	// Culls anything with GetPosition() --> glm::vec3, and GetRadius() --> float
	void Cull(std::vector<T>& cullables, std::vector<bool>& isCulledVector, const glm::mat4& viewProj)
	{
		isCulledVector.clear();
		isCulledVector.resize(cullables.size());

		glm::vec4 frustumPlanes[6];
		ExtractFrustumPlanes(viewProj, frustumPlanes);

		size_t numItems = cullables.size();
		for (size_t i = 0; i < numItems; ++i)
		{
			T& object = cullables[i];
			isCulledVector[i] = !InFrustum(frustumPlanes, object.GetPosition(), object.GetRadius());
		}
	}

	static bool InFrustum(const glm::vec4 frustumPlanes[], const glm::vec3 point, float radius)
	{
		const glm::vec4 p = glm::vec4(point, 1.0f);

		for (int i = 0; i < 6; ++i)
		{
			const glm::vec4 plane = frustumPlanes[i];
			float d = glm::dot(p, plane);
			if (d + radius < 0) 
				return false;
		}

		return true;
	}
};