#include "lupch.h"
#include "Functions.hpp"

#include "Lunar/Internal/IO/Print.hpp"
#include "Lunar/Internal/Utils/Profiler.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Lunar::Maths
{
	
	////////////////////////////////////////////////////////////////////////////////////
	// Matrices
	////////////////////////////////////////////////////////////////////////////////////
	Mat4 Perspective(float fov, float aspectRatio, float nearClip, float farClip)
	{
		return glm::perspective(fov, aspectRatio, nearClip, farClip);
	}

	Mat4 Orthographic(float aspectRatio, float zoom)
	{
		return Orthographic(-aspectRatio * zoom, aspectRatio * zoom, -1.0f * zoom, 1.0f * zoom, -1.0f, 1.0f);
	}

	Mat4 Orthographic(float left, float right, float bottom, float top, float nearClip, float farClip)
	{
		return glm::ortho(left, right, bottom, top, nearClip, farClip);
	}

	Mat4 LookAt(const Vec3<float>& position, const Vec3<float>& target, const Vec3<float>& up)
	{
		return glm::lookAt(position, target, up);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Utils
	///////////////////////////////////////////////////////////////////////////////////
	float Radians(float degrees)
	{
		return glm::radians(degrees);
	}

	float AspectRatio(uint32_t width, uint32_t height)
	{
		return static_cast<float>(width) / static_cast<float>(height);
	}

}