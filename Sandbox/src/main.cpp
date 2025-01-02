#include "Lunar/Enum/Name.hpp"

#include "Lunar/IO/Print.hpp"

enum class Colour
{
    Green,
    Red,
    Blue
};

////////////////////////////////////////////////////////////////////
// Impl
////////////////////////////////////////////////////////////////////
constexpr bool IsPretty(char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

constexpr std::string_view PrettyName(std::string_view sv)
{
    for (size_t n = sv.size() - 1; n > 0; --n) 
    {
        if (!IsPretty(sv[n])) 
        {
            sv.remove_prefix(n + 1);
            break;
        }
    }

    return sv;
}

template<typename TEnum, TEnum EValue>
constexpr std::string_view n()
{
    #if defined(__GNUC__) || defined(__clang__)
        return PrettyName({ __PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 2 });
    #elif defined(_MSC_VER)
        return PrettyName({ __FUNCSIG__, sizeof(__FUNCSIG__) - 17 });
    #endif
}
////////////////////////////////////////////////////////////////////

int main()
{
    LU_LOG_TRACE("{0}", n<Colour, Colour::Green>());
    LU_LOG_TRACE("{0}", n<Colour, Colour::Blue>());
    LU_LOG_TRACE("{0}", n<Colour, Colour::Red>());

    return 0;
}