#pragma once

#include <cstdint>
#include <cmath>
#include <type_traits>

#include "Lunar/Enum/Name.hpp"
#include "Lunar/Enum/Utilities.hpp"

#include "Lunar/Maths/Log.hpp"

#include "Lunar/Utils/Hash.hpp"

namespace Lunar::Enum::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Helper functions
    ////////////////////////////////////////////////////////////////////////////////////
    enum class FusedEnum : uintmax_t;

    template<typename TEnum> requires(std::is_enum_v<TEnum>)
    constexpr uintmax_t FuseOne(uintmax_t hash, TEnum value)
    {
        size_t index = Index(value);
        return (hash << Lunar::Log2((Count<TEnum>() << 1) - 1)) | index;
    }

    template<typename TEnum> requires(std::is_enum_v<TEnum>)
    constexpr uintmax_t Fuse(TEnum value)
    {
        return FuseOne(0, value);
    }

    template <typename TEnum, typename... TEnums> requires(std::is_enum_v<TEnum> && (std::is_enum_v<TEnums> && ...))
    constexpr uintmax_t Fuse(TEnum head, TEnums... tail)
    {
        return FuseOne(Fuse(tail...), head);
    }

}

namespace Lunar::Enum
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Public functions
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename... TEnums>
    constexpr Internal::FusedEnum Fuse(TEnums... values) requires((std::is_enum_v<TEnums> && ...))
    {
        static_assert(sizeof...(TEnums) >= 2, "Fuse requires at least 2 values.");
        return static_cast<Internal::FusedEnum>(Internal::Fuse<std::decay_t<TEnums>...>(values...));
    }

}