#include "lupch.h"
#include "BatchRenderer2D.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include "Lunar/Internal/Renderer/Shader.hpp"

namespace
{

	////////////////////////////////////////////////////////////////////////////////////
	// Helper functions
	////////////////////////////////////////////////////////////////////////////////////
	Lunar::Internal::BufferLayout GetVertexBufferLayout()
	{
		return {
			{ Lunar::Internal::DataType::Float3,	0, "Position",	Lunar::Internal::VertexInputRate::Vertex },
			{ Lunar::Internal::DataType::Float2,	1, "UV",		Lunar::Internal::VertexInputRate::Vertex },
			{ Lunar::Internal::DataType::Float4,	2, "Colour",	Lunar::Internal::VertexInputRate::Vertex },
			{ Lunar::Internal::DataType::UInt,		3, "TextureID", Lunar::Internal::VertexInputRate::Vertex },
		};
	}

}

namespace Lunar
{

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
	BatchResources2D::BatchResources2D()
	{
		InitGlobal();
		InitRenderer();
	}

	BatchResources2D::~BatchResources2D()
	{
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Private methods
	////////////////////////////////////////////////////////////////////////////////////
	void BatchResources2D::InitGlobal()
	{
	}

	void BatchResources2D::InitRenderer()
	{
	}

}