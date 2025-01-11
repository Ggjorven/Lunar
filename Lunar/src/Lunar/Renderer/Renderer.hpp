#pragma once

#include <cstdint>
#include <functional>

#include "Lunar/Core/Window.hpp"

namespace Lunar
{

    //using FreeFn = std::copyable_function<void()>;
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
        virtual void Free(const FreeFn& fn) = 0;
        
        // TODO: Submit

        virtual void Recreate(uint32_t width, uint32_t height, bool vsync) = 0;

        // Getters
        virtual RendererID GetID() const = 0;
        virtual const RendererSpecification& GetSpecification() const = 0;

        // Static methods
        static Arc<Renderer> Create(const RendererSpecification& specs);
    };

}