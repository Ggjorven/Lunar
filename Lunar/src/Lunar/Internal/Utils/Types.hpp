#pragma once

#include <iostream>
#include <string_view>
#include <cstdint>
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
    constexpr const std::string_view g_InvalidName = "1nvalid";

    template<typename T>
    struct ConstexprName
    {
    private:
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

    public:
        constexpr static std::string_view Name(std::string_view fn)
        {
            #if defined(LU_COMPILER_MSVC) || defined(LU_COMPILER_CLANG)
                constexpr std::string_view startSeq = "ConstexprName<";
                constexpr std::string_view endSeq = ">::FullName";

                size_t start = fn.find(startSeq);
                if (start == std::string_view::npos)
                    return g_InvalidName;
                start += startSeq.size();

                size_t end = fn.rfind(endSeq);
                if (end == std::string_view::npos) 
                    return g_InvalidName;

                std::string_view typeName = fn.substr(start, end - start);
                size_t spacePos = typeName.find(' ');
                if (spacePos != std::string_view::npos)
                    typeName = typeName.substr(spacePos + 1); // Move past the space

                return typeName;
            #elif defined(LU_COMPILER_GCC)
                constexpr std::string_view startSeq = "with T = ";
                constexpr std::string_view endSeq = ";";

                size_t start = fn.find(startSeq);
                if (start == std::string_view::npos) 
                    return g_InvalidName;
                start += startSeq.size();

                size_t end = fn.rfind(endSeq);
                if (end == std::string_view::npos) 
                    return g_InvalidName;

                return fn.substr(start, end - start);
            #else
                #error Lunar Types: Unsupported compiler...
            #endif
        }

        constexpr static const std::string_view TypeName = Name(FullName());
    };

}