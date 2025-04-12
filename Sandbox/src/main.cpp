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
		Texture texture(renderer.GetID(), "Resources/Images/texture.jpg");
		//Texture texture(renderer.GetID(), "Resources/Images/Mario.png");

		{
			double currentTime = window.GetTime();
			double deltaTime = 0.0f;

			float cameraDistance = 3.0f;
			float cameraSpeed = 0.5f;

			float aspectRatio = Maths::AspectRatio(window.GetSize().x, window.GetSize().y);

			Mat4 view = Mat4(1.0f);
			//Mat4 projection = Mat4(1.0f);
			Mat4 projection = Maths::Perspective(Maths::Radians(45.0f), aspectRatio, 0.1f, 100.0f);
			projection[1][1] *= -1.0f;

			while (window.IsOpen())
			{
				window.PollEvents();
				renderer.BeginFrame();

				// Rotating camera position
				float angle = static_cast<float>(currentTime) * cameraSpeed;
				Vec3<float> cameraPos = Vec3<float>(Maths::Sin(angle) * cameraDistance, 1.5f, Maths::Cos(angle) * cameraDistance);

				view = Maths::LookAt(cameraPos, { 0.0f, 0.0f, 0.0f });

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