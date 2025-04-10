#include "Lunar/Core/Window.hpp"

#include "Lunar/Maths/Functions.hpp"

using namespace Lunar;

int main(int argc, char* argv[])
{
	Window window({
		.Title = "Sandbox",

		.Width = 1280,
		.Height = 720,

		.VSync = false,
	});
	Renderer& renderer = window.GetRenderer();

	{
		Texture texture(renderer.GetID(), 1, 1);
		size_t colour = 0xFF00FF00;
		texture.SetData(&colour, sizeof(size_t));

		{
			double currentTime = window.GetTime();
			double deltaTime = 0.0f;

			float cameraDistance = 3.0f;
			float cameraSpeed = 0.5f;

			float aspectRatio = Maths::AspectRatio(window.GetSize().x, window.GetSize().y);

			Mat4 view = Mat4(1.0f);
			Mat4 projection = Maths::Perspective(Maths::Radians(45.0f), aspectRatio, 0.1f, 100.0f);

			while (window.IsOpen())
			{
				window.PollEvents();
				renderer.BeginFrame();

				// Rotating camera position
				float angle = static_cast<float>(currentTime) * cameraSpeed;
				Vec3<float> cameraPos = Vec3<float>(Maths::Sin(angle) * cameraDistance, 1.5f, Maths::Cos(angle) * cameraDistance);
				Vec3<float> target = Vec3<float>(0.0f, 0.0f, 0.0f);
				Vec3<float> up = Vec3<float>(0.0f, 1.0f, 0.0f);

				Mat4 view = Maths::LookAt(cameraPos, target, up);

				renderer.Set2DCamera(view, projection);
				renderer.DrawQuad({ 0.0f, 0.0f, 0.0f }, { 0.5f, 0.5f }, texture);

				renderer.EndFrame();
				window.SwapBuffers();

				deltaTime = window.GetTime() - currentTime;
				currentTime = window.GetTime();
			}
		}
	}

    return 0;
}