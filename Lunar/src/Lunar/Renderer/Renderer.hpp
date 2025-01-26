#pragma once

#include <cstdint>
#include <functional>

#include "Lunar/Core/Window.hpp"

#include "Lunar/Enum/Bitwise.hpp"

namespace Lunar
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Configurations
    ////////////////////////////////////////////////////////////////////////////////////
    enum class ExecutionPolicy
    {
        InOrder = 1 << 0,           // Execute commands sequentially, submits to waited on by next (WaitForPrevious) commandBuffer
        Parallel = 1 << 1,          // Execute commands in parallel but synchronized by the frame

        WaitForPrevious = 1 << 2,   // Wait for the completion of the previous (InOrder) command buffer
    };
    LU_ENABLE_BITWISE(ExecutionPolicy)

    using FreeFn = std::function<void()>;

    ////////////////////////////////////////////////////////////////////////////////////
    // RendererSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct RendererSpecification
    {
    public:
        Window& WindowRef;

        uint32_t Width = 0, Height = 0;

        WindowSpecification::BufferMode Buffers = WindowSpecification::BufferMode::Triple;
        bool VSync = true;
    };

	using RendererID = uint8_t;

    ////////////////////////////////////////////////////////////////////////////////////
    // Renderer
    ////////////////////////////////////////////////////////////////////////////////////
    class Renderer
    {
    public:
        // Constructor & Destructor
        Renderer() = default;
        virtual ~Renderer() = default;
      
        // Methods
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void Present() = 0;

        virtual void Free(const FreeFn& fn) = 0;
        virtual void FreeQueue() = 0;
        
        // TODO: Submit

        virtual void Recreate(uint32_t width, uint32_t height, bool vsync) = 0;

        // Getters
        virtual RendererID GetID() const = 0;
        virtual const RendererSpecification& GetSpecification() const = 0;

        // Static methods
        static Arc<Renderer> Create(const RendererSpecification& specs);
    };

}