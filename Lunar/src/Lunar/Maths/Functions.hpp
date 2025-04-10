#pragma once

#include "Lunar/Maths/Structs.hpp"

#include <cstdint>
#include <type_traits>

namespace Lunar::Maths
{

	////////////////////////////////////////////////////////////////////////////////////
	// Matrices
	////////////////////////////////////////////////////////////////////////////////////
	Mat4 Perspective(float fov, float aspectRatio, float nearClip = 0.1f, float farClip = 100.0f);

	Mat4 Orthographic(float aspectRatio, float zoom = 1.0f);
	Mat4 Orthographic(float left, float right, float bottom, float top, float nearClip = -1.0f, float farClip = 1.0f);

	Mat4 LookAt(const Vec3<float>& position, const Vec3<float>& target, const Vec3<float>& up = { 0.0f, 1.0f, 0.0f });

	////////////////////////////////////////////////////////////////////////////////////
	// Trigonometry 
	////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline float Sin(T angle) { return glm::sin(angle); }
	template<typename T>
	inline float Cos(T angle) { return glm::cos(angle); }
	template<typename T>
	inline float Tan(T angle) { return glm::tan(angle); }

	////////////////////////////////////////////////////////////////////////////////////
	// Utils
	////////////////////////////////////////////////////////////////////////////////////
	float Radians(float degrees);
	float AspectRatio(uint32_t width, uint32_t height);

}