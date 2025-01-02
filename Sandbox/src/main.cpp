#include "Lunar/Enum/Name.hpp"

#include "Lunar/IO/Print.hpp"

enum class Colour
{
    Green = 7,
    Red,
    Blue = 20
};

int main()
{
    // Name tests
    LU_LOG_TRACE("Name: {0}", Lunar::Enum::Internal::Name<Colour, Colour::Green>());
    LU_LOG_TRACE("Name: {0}", Lunar::Enum::Internal::Name<Colour, Colour::Blue>());
    LU_LOG_TRACE("Name: {0}", Lunar::Enum::Internal::Name<Colour, Colour::Red>());
    LU_LOG_TRACE("Name: {0}", Lunar::Enum::Internal::Name<Colour, static_cast<Colour>(127)>());

    // Validity
    LU_LOG_TRACE("Valid: {0}", Lunar::Enum::Internal::IsValid<Colour, Colour::Green>());
    LU_LOG_TRACE("Valid: {0}", Lunar::Enum::Internal::IsValid<Colour, static_cast<Colour>(127)>());

    // Values
    auto valueList = Lunar::Enum::Internal::Values<Colour>;
    for (const auto& v : valueList)
    {
        LU_LOG_TRACE("Value: {0}", static_cast<typename std::underlying_type_t<Colour>>(v)); 
    }

    // Entries
    auto entryList = Lunar::Enum::Internal::Entries<Colour>;
    for (const auto& [colour, name] : entryList)
    {
        LU_LOG_TRACE("Entry: {0} - {1}", static_cast<typename std::underlying_type_t<Colour>>(colour), name);
    }

    return 0;
}