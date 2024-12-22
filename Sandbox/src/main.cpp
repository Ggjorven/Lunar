#include <iostream>

#include "Lunar/IO/Print.hpp"

int main(int argc, char* argv[])
{
    LU_LOG_TRACE("HI {0}", 1);
    LU_LOG_INFO("HI {0}", 1);
    LU_LOG_WARN("HI {0}", 1);
    LU_LOG_ERROR("HI {0}", 1);
    LU_LOG_FATAL("HI {0}", 1);

    LU_VERIFY(false, "Message");
    LU_ASSERT(false, "Message");

    return 0;
}