#pragma once

#include <cstdint>
#include <set>
#include <map>
#include <array>
#include <utility>
#include <optional>
#include <algorithm>
#include <string_view>
#include <type_traits>

namespace Lunar::Enum
{

    constexpr const int32_t g_MinValue = -128;
    constexpr const int32_t g_MaxValue = 128;
    constexpr const std::string_view g_InvalidName = "1nvalid";

    #if defined(_MSC_VER)

    template <typename TEnum, TEnum EValue> requires(std::is_enum_v<TEnum>)
    class ConstexprName
    {
    public:
        constexpr static const std::string_view ClassToken = "Lunar::Enum::ConstexprName<";

    public:
        template <TEnum>
        constexpr static std::string_view FullNameImpl()
        {
            constexpr std::string_view FunctionToken = ">::FullNameImpl<";

            // Function signature
            constexpr std::string_view function = __FUNCSIG__;

            // Class token
            constexpr auto startClassToken = function.find(ClassToken);
            constexpr auto endClassToken = startClassToken + ClassToken.size();
            if (startClassToken == std::string_view::npos)
                return g_InvalidName;

            // Function token
            constexpr auto startFunctionToken = function.find(FunctionToken, endClassToken);
            constexpr auto endFunctionToken = startFunctionToken + FunctionToken.size();
            if (startFunctionToken == std::string_view::npos)
                return g_InvalidName;

            // Close marker
            constexpr auto closeMarker = function.find('>', endFunctionToken);
            if (closeMarker == std::string_view::npos)
                return g_InvalidName;

            // Full name
            constexpr std::string_view fullName = function.substr(endFunctionToken, closeMarker - endFunctionToken);

            // Brackets (if brackets, then not a valid enum name, ex. (enum Dummy)0xa instead of Dummy::First)
            if (fullName.find('(') != std::string_view::npos)
                return g_InvalidName;

            return fullName;
        }

        constexpr static std::string_view ElementNameImpl()
        {
            constexpr std::string_view fullName = FullNameImpl<EValue>();

            // Check for invalid name
            if (!fullName.compare(g_InvalidName))
                return g_InvalidName;

            // '::' marker
            constexpr auto startColon = fullName.find("::");
            constexpr auto endColon = startColon + std::string_view("::").size();
            if (startColon == std::string_view::npos)
                return g_InvalidName;

            // Element name
            constexpr std::string_view elementName = fullName.substr(endColon, fullName.size() - endColon);

            return elementName;
        }

    public:
        constexpr static const std::string_view FullName = FullNameImpl<EValue>();
        constexpr static const std::string_view ElementName = ElementNameImpl();
    };

    #elif defined(__GNUC__) || defined(__clang__)
        #error Enum not implemented on this compiler.
    #endif

    class Internal
    {
    public:
        constexpr static bool Valid(const std::string_view str) 
        { 
            return !str.compare(g_InvalidName);
        }

        template<typename TEnum>
        constexpr static TEnum UAlue(size_t value) requires(std::is_enum_v<TEnum>)
        {
            return static_cast<TEnum>(g_MinValue + value);
        }

        template<size_t N>
        constexpr static size_t CountValues(const bool (&valid)[N])
        {
            size_t count = 0;

            for (size_t n = 0; n < N; ++n)
            {
                if (valid[n])
                    ++count;
            }

            return count;
        }

    public:
        template<typename TEnum, size_t... I>
        constexpr static auto Values(std::index_sequence<I...>) requires(std::is_enum_v<TEnum>)
        {
            constexpr bool valid[sizeof...(I)] = { Valid(ConstexprName<TEnum, UAlue<TEnum>(I)>::ElementName)... };
            constexpr size_t numberOfValid = CountValues(valid);

            static_assert(numberOfValid > 0, "No support for empty enums.");

            std::array<TEnum, numberOfValid> values;

            for (size_t offset = 0, n = 0; n < numberOfValid; offset++) 
            {
                if (valid[offset]) 
                {
                    values[n] = UAlue<TEnum>(offset);
                    ++n;
                }
            }

            return values;
        }

        template<typename TEnum>
        constexpr static auto Values()
        {
            constexpr auto enumSize = g_MaxValue - g_MinValue + 1;
            return Values<TEnum>(std::make_index_sequence<enumSize>({}));
        }
    };

}