#pragma once

#include <cstdint>

#include "Lunar/Internal/Enum/Fuse.hpp"

namespace Lunar::Enum
{

    enum class FusedEnum : uintmax_t;

    ////////////////////////////////////////////////////////////////////////////////////
    // Public functions
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename... TEnums>
    constexpr FusedEnum Fuse(TEnums... values) requires((std::is_enum_v<TEnums> && ...))
    {
        static_assert(sizeof...(TEnums) >= 2, "Fuse requires at least 2 values.");
        return static_cast<FusedEnum>(Internal::Enum::Fuse<std::decay_t<TEnums>...>(values...));
    }

}