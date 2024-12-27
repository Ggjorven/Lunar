#pragma once

#include <variant>
#include <type_traits>

namespace Lunar::Types
{

    template <typename T, typename Variant>
    concept TypeInVariant = requires 
    {
        []<typename ...Types>(std::variant<Types...> obj) 
        {
            return (std::is_same_v<T, Types> || ...);
        }
        (std::declval<Variant>());
    };

}