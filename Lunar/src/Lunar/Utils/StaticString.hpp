#pragma once

#include <cstdint>
#include <array>
#include <utility>
#include <string_view>

namespace Lunar
{

    ////////////////////////////////////////////////////////////////////////////////////
    // StaticString
    ////////////////////////////////////////////////////////////////////////////////////
	template<size_t N>
    class StaticString
    {
    public:
        // Constructor
        constexpr StaticString(std::string_view sv) 
        {
            std::copy(sv.data(), m_Contents.data());
        }

        // Operators
        constexpr operator std::string_view() const { return { m_Contents.data(), N }; }

    private:
        std::array<char, N + 1> m_Contents;
    };

}