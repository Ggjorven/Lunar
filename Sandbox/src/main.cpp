///////////////////////////////////////////////////////////
// Build test file
///////////////////////////////////////////////////////////
#include "Lunar/Internal/Core/Events.hpp"

#include "Lunar/Internal/Enum/Bitwise.hpp"
#include "Lunar/Internal/Enum/Fuse.hpp"
#include "Lunar/Internal/Enum/Name.hpp"
#include "Lunar/Internal/Enum/Utilities.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include "Lunar/Internal/Maths/Logarithm.hpp"
#include "Lunar/Internal/Maths/Structs.hpp"

#include "Lunar/Internal/Memory/Arc.hpp"
#include "Lunar/Internal/Memory/AutoRelease.hpp"
#include "Lunar/Internal/Memory/Box.hpp"
#include "Lunar/Internal/Memory/Rc.hpp"

#include "Lunar/Internal/Utils/Hash.hpp"
#include "Lunar/Internal/Utils/Preprocessor.hpp"
#include "Lunar/Internal/Utils/Profiler.hpp"
#include "Lunar/Internal/Utils/Types.hpp"

#include "Lunar/Enum/Name.hpp"
#include "Lunar/Enum/Fuse.hpp"

#include "Lunar/Maths/Structs.hpp"

///////////////////////////////////////////////////////////
// Test Enum
///////////////////////////////////////////////////////////
enum class Test : uint8_t
{
    Hi = 0,
    Hello = 27,
    Hey,
};

///////////////////////////////////////////////////////////
// Test main
///////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // Enum Test
    constexpr const Test compValue = Test::Hello;
    volatile const Test runtimeValue = Test::Hey;

    constexpr std::string_view compValueStr = Lunar::Enum::Name(compValue);
    LU_LOG_TRACE("Compile time: {0}", compValueStr);
    LU_LOG_TRACE("Runtime: {0}", Lunar::Enum::Name(runtimeValue));

    return 0;
}