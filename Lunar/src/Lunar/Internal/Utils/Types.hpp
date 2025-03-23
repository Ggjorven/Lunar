#pragma once

#include <variant>
#include <type_traits>

#include "Lunar/Internal/Utils/Settings.hpp"

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

    ////////////////////////////////////////////////////////////////////////////////////
	// Helper functions
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    struct ConstexprName 
    {
    public:
        constexpr static std::string_view FullName() 
        {
            #if defined(LU_COMPILER_MSVC)
                return __FUNCSIG__;
            #elif defined(LU_COMPILER_GCC) || defined(LU_COMPILER_CLANG)
                return __PRETTY_FUNCTION__;
            #else
                #error Lunar Types: Unsupported compiler...
            #endif
        }

        constexpr static std::string_view Name() 
        {
            constexpr std::string_view fn = FullName();
            constexpr std::string_view start_seq = "ConstexprName<";
            constexpr std::string_view end_seq = ">::FullName";

            constexpr size_t start = fn.find(start_seq) + start_seq.size();
            constexpr size_t end = fn.rfind(end_seq);
            return fn.substr(start, end - start);
        }

    public:
        constexpr static const std::string_view TypeName = Name(); // TODO: Fix on Apple Clang
    };

}