#pragma once

#include <cstdint>

#include "Lunar/Enum/Name.hpp"

namespace Lunar::Enum
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Public functions
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename TEnum> requires(std::is_enum_v<TEnum>)
    constexpr size_t Count()
    {
        constexpr auto values = Internal::Entries<TEnum>;
        return values.size();
    }

    template<typename TEnum> requires(std::is_enum_v<TEnum>)
    constexpr size_t Index(const TEnum value)
    {
        constexpr auto values = Internal::Entries<TEnum>;

        size_t index = 0;
        for (const auto& [v, n] : values)
        {
            if (v == value)
                return index;

            index++;
        }

        return index;
    }

}