#pragma once

#include <variant>
#include <type_traits>

namespace Lunar::Internal::Types
{

	////////////////////////////////////////////////////////////////////////////////////
    // Concepts
    ////////////////////////////////////////////////////////////////////////////////////
    template <typename T, typename Variant>
    struct IsTypeInVariant;

    template <typename T, typename... Types>
    struct IsTypeInVariant<T, std::variant<Types...>> : std::bool_constant<(std::is_same_v<T, Types> || ...)> {};

    template <typename T, typename Variant>
    concept TypeInVariant = IsTypeInVariant<T, Variant>::value;

}