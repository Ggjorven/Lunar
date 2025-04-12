#pragma once

#include <cstdint>
#include <cmath>
#include <type_traits>

#include "Lunar/Internal/Enum/Name.hpp"
#include "Lunar/Internal/Enum/Utilities.hpp"

#include "Lunar/Internal/Utils/Hash.hpp"
#include "Lunar/Internal/Utils/Types.hpp"

namespace Lunar::Internal::Enum
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Helper functions
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename TEnum> requires(std::is_enum_v<TEnum>)
    constexpr uintmax_t FuseOne(uintmax_t hash, TEnum value)
    {
		constexpr auto typeHash = Hash::fnv1a(Types::ConstexprName<TEnum>::TypeName);
        uintmax_t integerValue = static_cast<uintmax_t>(value);

        return (hash * 31) ^ (typeHash + integerValue);
    }

    template<typename TEnum> requires(std::is_enum_v<TEnum>)
    constexpr uintmax_t Fuse(TEnum value)
    {
        return FuseOne(0, value);
    }

    template<typename TEnum, typename... TEnums> requires(std::is_enum_v<TEnum> && (std::is_enum_v<TEnums> && ...))
    constexpr uintmax_t Fuse(TEnum head, TEnums... tail)
    {
        return FuseOne(Fuse(tail...), head);
    }

}