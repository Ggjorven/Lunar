#pragma once

#include <variant>
#include <type_traits>

namespace Lunar::Internal::Types
{

	////////////////////////////////////////////////////////////////////////////////////
    // Concepts
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename T, typename Variant>
    concept TypeInVariant = requires 
    {
        []<typename ...Types>(std::variant<Types...>) 
        {
            return (std::is_same_v<T, Types> || ...);
        }
        (std::declval<Variant>());
    };

}