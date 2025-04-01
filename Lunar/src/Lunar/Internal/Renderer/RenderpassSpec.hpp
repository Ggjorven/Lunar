#pragma once

#include "Lunar/Internal/Enum/Bitwise.hpp"

#include "Lunar/Internal/Renderer/ImageSpec.hpp"

#include "Lunar/Maths/Structs.hpp"

#include <cstdint>
#include <vector>

namespace Lunar::Internal
{

    class Image;

    ////////////////////////////////////////////////////////////////////////////////////
    // Renderpass specs
    ////////////////////////////////////////////////////////////////////////////////////
    enum class LoadOperation : uint8_t
    {
        None = 0,
        Load,
        Clear,
        DontCare,

        NoneKHR = None,
        NoneEXT = None,
    };

	enum class StoreOperation : uint8_t
    {
        None = 0,
        Store,
        DontCare,

        NoneKHR = None,
        NoneQCOM = None,
        NoneEXT = None,
    };

    enum class RenderpassUsage : uint8_t
    {
        None = 0,
        Forward, Graphics = Forward,
        ComputeToTexture, 
        Offscreen, GBuffer = Offscreen
    };

    struct RenderpassSpecification
    {
    public:
        // General
        RenderpassUsage Usage = RenderpassUsage::None;

        // Colour
        std::vector<Image*> ColourAttachment = { }; // This is a vector since the swapchain has multiple images (most of the time).
        LoadOperation ColourLoadOp = LoadOperation::Clear;
        StoreOperation ColourStoreOp = StoreOperation::Store;
        Vec4<float> ColourClearColour = { 0.0f, 0.0f, 0.0f, 1.0f };
        ImageLayout PreviousColourImageLayout = ImageLayout::Undefined;
        ImageLayout FinalColourImageLayout = ImageLayout::PresentSrcKHR;

        // Depth
        Image* DepthAttachment = nullptr;
        LoadOperation DepthLoadOp = LoadOperation::Clear;
        StoreOperation DepthStoreOp = StoreOperation::Store;
        ImageLayout PreviousDepthImageLayout = ImageLayout::Undefined;
        ImageLayout FinalDepthImageLayout = ImageLayout::DepthStencil;
    };

}