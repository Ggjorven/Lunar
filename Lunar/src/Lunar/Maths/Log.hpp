#pragma once

#include <cstdint>
#include <cmath>
#include <type_traits>

namespace Lunar
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Logarithmic operations
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename Integral>
    constexpr Integral Log2(Integral value) requires(std::is_integral_v<Integral>)
    {
        auto ret = Integral(0);
        for (; value > Integral(1); value >>= Integral(1), ret++) {}

        return ret;
    }

}