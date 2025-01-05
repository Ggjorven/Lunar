#pragma once

#include <cstdint>

namespace Lunar
{

    ////////////////////////////////////////////////////////////////////////////////////
    // GraphicsContext
	////////////////////////////////////////////////////////////////////////////////////
	// Note: You only need one context for an entire application, even with multiple windows
	// So, only initialize it once.
	class GraphicsContext
	{
    public:
		// Constructors & Destructor
        GraphicsContext() = default;
        virtual ~GraphicsContext() = default;

		// Getters
        static bool Initialized();

		// Static methods
        static void AttachWindow(void* nativeWindow);
		
		static void Init();
		static void Destroy();
	};

}