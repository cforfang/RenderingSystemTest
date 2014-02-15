#include "FrustumCuller.h"

#include "glm/glm.hpp"

static float InvSqrt(float x)
{
	float xhalf;
	int i;

	xhalf = 0.5f * x;
	i = *(int*)&x;
	i = 0x5f3759df - (i >> 1);
	x = *(float*)&i;
	x = x * (1.5f - xhalf * x * x);
	return x;
}

// http://www.gamedev.net/topic/640709-issue-with-frustum-extraction-using-glm/
void ExtractFrustumPlanes(const glm::mat4& viewProj, glm::vec4 planes[6])
{
	float t;
	float m00, m01, m02, m03;
	float m10, m11, m12, m13;
	float m20, m21, m22, m23;
	float m30, m31, m32, m33;

	glm::mat4 mvp = glm::transpose(viewProj);

	m00 = mvp[0][0];
	m01 = mvp[0][1];
	m02 = mvp[0][2];
	m03 = mvp[0][3];

	m10 = mvp[1][0];
	m11 = mvp[1][1];
	m12 = mvp[1][2];
	m13 = mvp[1][3];

	m20 = mvp[2][0];
	m21 = mvp[2][1];
	m22 = mvp[2][2];
	m23 = mvp[2][3];

	m30 = mvp[3][0];
	m31 = mvp[3][1];
	m32 = mvp[3][2];
	m33 = mvp[3][3];

	planes[0].x = m30 - m00;
	planes[0].y = m31 - m01;
	planes[0].z = m32 - m02;
	planes[0].w = m33 - m03;

	t = InvSqrt(planes[0].x * planes[0].x + planes[0].y * planes[0].y + planes[0].z * planes[0].z);

	planes[0].x *= t;
	planes[0].y *= t;
	planes[0].z *= t;
	planes[0].w *= t;

	planes[1].x = m30 + m00;
	planes[1].y = m31 + m01;
	planes[1].z = m32 + m02;
	planes[1].w = m33 + m03;

	t = InvSqrt(planes[1].x * planes[1].x + planes[1].y * planes[1].y + planes[1].z * planes[1].z);

	planes[1].x *= t;
	planes[1].y *= t;
	planes[1].z *= t;
	planes[1].w *= t;

	planes[2].x = m30 - m10;
	planes[2].y = m31 - m11;
	planes[2].z = m32 - m12;
	planes[2].w = m33 - m13;

	t = InvSqrt(planes[2].x * planes[2].x + planes[2].y * planes[2].y + planes[2].z * planes[2].z);

	planes[2].x *= t;
	planes[2].y *= t;
	planes[2].z *= t;
	planes[2].w *= t;

	planes[3].x = m30 + m10;
	planes[3].y = m31 + m11;
	planes[3].z = m32 + m12;
	planes[3].w = m33 + m13;

	t = InvSqrt(planes[3].x * planes[3].x + planes[3].y * planes[3].y + planes[3].z * planes[3].z);

	planes[3].x *= t;
	planes[3].y *= t;
	planes[3].z *= t;
	planes[3].w *= t;

	planes[4].x = m30 - m20;
	planes[4].y = m31 - m21;
	planes[4].z = m32 - m22;
	planes[4].w = m33 - m23;

	t = InvSqrt(planes[4].x * planes[4].x + planes[4].y * planes[4].y + planes[4].z * planes[4].z);

	planes[4].x *= t;
	planes[4].y *= t;
	planes[4].z *= t;
	planes[4].w *= t;

	planes[5].x = m30 + m20;
	planes[5].y = m31 + m21;
	planes[5].z = m32 + m22;
	planes[5].w = m33 + m23;

	t = InvSqrt(planes[5].x * planes[5].x + planes[5].y * planes[5].y + planes[5].z * planes[5].z);

	planes[5].x *= t;
	planes[5].y *= t;
	planes[5].z *= t;
	planes[5].w *= t;
}
