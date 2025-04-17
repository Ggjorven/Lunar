#include "SandboxApp/SandboxApp.hpp"

int main(int argc, char* argv[])
{
	{
		SandboxApp app = {};

		while (app.Running())
		{
			app.Update();
			app.Render();
		}
	}

    return 0;
}