#include "Lunar/Enum/Name.hpp"
#include "Lunar/Enum/Fuse.hpp"

#include "Lunar/IO/Print.hpp"

enum class Colour : uint32_t
{
    Green = 1,
    Red,
    Blue,
    Pink,
};

enum class Direction : uint32_t
{
    Up = 1,
    Down,
    Left,
    Right
};

int main(int argc, char* argv[])
{
    LU_LOG_TRACE("{0}", Lunar::Enum::Name(Colour::Pink));
    return 0;
}