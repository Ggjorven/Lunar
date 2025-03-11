#pragma once

#include <cstdint>
#include <cstdlib>
#include <print>
#include <string>
#include <string_view>
#include <format>
#include <chrono>
#include <ctime>

#include <iostream>
#include <iomanip>
#include <sstream>

#if defined(LU_PLATFORM_WINDOWS)
    #include <intrin.h>
    #define LU_DEBUG_BREAK() __debugbreak()
#elif defined(LU_PLATFORM_LINUX) || defined(LU_PLATFORM_MACOS)
    #include <csignal>
    #define LU_DEBUG_BREAK() std::raise(SIGTRAP)
#else
    #include <cstdlib>
    #define LU_DEBUG_BREAK() std::abort()
#endif

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Colours
    ////////////////////////////////////////////////////////////////////////////////////
	namespace Log::Colour
	{
        // Reset
        inline constexpr std::string_view Reset = "\033[0m";

        // Text Attributes
        inline constexpr std::string_view Bold = "\033[1m";
        inline constexpr std::string_view Dim = "\033[2m";
        inline constexpr std::string_view Italic = "\033[3m";
        inline constexpr std::string_view Underline = "\033[4m";
        inline constexpr std::string_view BlinkSlow = "\033[5m";
        inline constexpr std::string_view BlinkFast = "\033[6m";
        inline constexpr std::string_view Invert = "\033[7m";
        inline constexpr std::string_view Hidden = "\033[8m";
        inline constexpr std::string_view StrikeThrough = "\033[9m";

        // Foreground Colors
        inline constexpr std::string_view BlackFG = "\033[30m";
        inline constexpr std::string_view RedFG = "\033[31m";
        inline constexpr std::string_view GreenFG = "\033[32m";
        inline constexpr std::string_view YellowFG = "\033[33m";
        inline constexpr std::string_view BlueFG = "\033[34m";
        inline constexpr std::string_view MagentaFG = "\033[35m";
        inline constexpr std::string_view CyanFG = "\033[36m";
        inline constexpr std::string_view WhiteFG = "\033[37m";

        // Bright Foreground Colors
        inline constexpr std::string_view BrightBlackFG = "\033[90m";
        inline constexpr std::string_view BrightRedFG = "\033[91m";
        inline constexpr std::string_view BrightGreenFG = "\033[92m";
        inline constexpr std::string_view BrightYellowFG = "\033[93m";
        inline constexpr std::string_view BrightBlueFG = "\033[94m";
        inline constexpr std::string_view BrightMagentaFG = "\033[95m";
        inline constexpr std::string_view BrightCyanFG = "\033[96m";
        inline constexpr std::string_view BrightWhiteFG = "\033[97m";

        // Background Colors
        inline constexpr std::string_view BlackBG = "\033[40m";
        inline constexpr std::string_view RedBG = "\033[41m";
        inline constexpr std::string_view GreenBG = "\033[42m";
        inline constexpr std::string_view YellowBG = "\033[43m";
        inline constexpr std::string_view BlueBG = "\033[44m";
        inline constexpr std::string_view MagentaBG = "\033[45m";
        inline constexpr std::string_view CyanBG = "\033[46m";
        inline constexpr std::string_view WhiteBG = "\033[47m";

        // Bright Background Colors
        inline constexpr std::string_view BrightBlackBG = "\033[100m";
        inline constexpr std::string_view BrightRedBG = "\033[101m";
        inline constexpr std::string_view BrightGreenBG = "\033[102m";
        inline constexpr std::string_view BrightYellowBG = "\033[103m";
        inline constexpr std::string_view BrightBlueBG = "\033[104m";
        inline constexpr std::string_view BrightMagentaBG = "\033[105m";
        inline constexpr std::string_view BrightCyanBG = "\033[106m";
        inline constexpr std::string_view BrightWhiteBG = "\033[107m";
	}

    ////////////////////////////////////////////////////////////////////////////////////
    // Loggging
    ////////////////////////////////////////////////////////////////////////////////////
	namespace Log
	{
        ////////////////////////////////////////////////////////////////////////////////////
        // Print
        ////////////////////////////////////////////////////////////////////////////////////
        template <typename... TArgs>
        void Print(std::string_view msg)
        {
            std::cout << msg << Colour::Reset;
        }

		template<typename ...TArgs>
		void PrintF(std::format_string<TArgs...> fmt, TArgs&&... args)
		{
            std::cout << std::format(fmt, std::forward<TArgs>(args)...) << Colour::Reset;
		}

        template<typename... TArgs>
        void PrintLn(std::format_string<TArgs...> fmt, TArgs&&... args)
        {
            PrintF(fmt, std::forward<TArgs>(args)...);
            std::cout << '\n';
        }

        ////////////////////////////////////////////////////////////////////////////////////
        // Levels
        ////////////////////////////////////////////////////////////////////////////////////
		enum class Level : uint8_t { Trace, Info, Warn, Error, Fatal };
        
        template<Level level>
        inline std::string LevelTag()
        {
            if constexpr (level == Level::Trace)
                return "T";
            else if constexpr (level == Level::Info)
                return "I";
            else if constexpr (level == Level::Warn)
                return "W";
            else if constexpr (level == Level::Error)
                return "E";
            else if constexpr (level == Level::Fatal)
                return "F";

            //return "Unset Tag";
        }

        template<Level level, typename ...TArgs>
        void PrintLvl(std::format_string<TArgs...> fmt, TArgs&&... args)
        {
            std::string tag = LevelTag<level>();
            std::string time;
            {
                auto now = std::chrono::system_clock::now();
                std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

                std::tm localTime = *std::localtime(&nowTime);

                std::ostringstream oss;
                oss << std::put_time(&localTime, "%H:%M:%S");
                time = oss.str();
            }

            std::string message = std::format("[{0}] [{1}]: {2}", time, tag, std::format(fmt, std::forward<TArgs>(args)...));

            // Set colour
            if constexpr (level == Level::Trace)
                std::cout << Colour::Reset;
            else if constexpr (level == Level::Info)
                std::cout << Colour::GreenFG;
            else if constexpr (level == Level::Warn)
                std::cout << Colour::BrightYellowFG;
            else if constexpr (level == Level::Error)
                std::cout << Colour::BrightRedFG;
            else if constexpr (level == Level::Fatal)
                std::cout << Colour::RedBG;
            
            // Simulate PrintLn
            Print(message);
            std::cout << '\n';
        }
    };

    #ifndef LU_CONFIG_DIST
        #define LU_LOG_TRACE(...)       ::Lunar::Internal::Log::PrintLvl<::Lunar::Internal::Log::Level::Trace>(__VA_ARGS__)
        #define LU_LOG_INFO(...)        ::Lunar::Internal::Log::PrintLvl<::Lunar::Internal::Log::Level::Info>(__VA_ARGS__)
        #define LU_LOG_WARN(...)        ::Lunar::Internal::Log::PrintLvl<::Lunar::Internal::Log::Level::Warn>(__VA_ARGS__)
        #define LU_LOG_ERROR(...)       ::Lunar::Internal::Log::PrintLvl<::Lunar::Internal::Log::Level::Error>(__VA_ARGS__)
        #define LU_LOG_FATAL(...)       ::Lunar::Internal::Log::PrintLvl<::Lunar::Internal::Log::Level::Fatal>(__VA_ARGS__)

        #define LU_ASSERT(x, msg)       \
            do                          \
            {                           \
                if (!x)                 \
                {                       \
                    LU_LOG_FATAL("Assertion failed: ({0}), {1}.", #x, msg); \
                    LU_DEBUG_BREAK();   \
                }                       \
            } while (false)

        #define LU_VERIFY(x, msg)       \
            do                          \
            {                           \
                if (!x)                 \
                {                       \
                    LU_LOG_FATAL("Verify failed: ({0}), {1}.", #x, msg); \
                }                       \
            } while (false)
        
    #else
        #define LU_LOG_TRACE(...) 
        #define LU_LOG_INFO(...) 
        #define LU_LOG_WARN(...) 
        #define LU_LOG_ERROR(...) 
        #define LU_LOG_FATAL(...) 

        #define LU_ASSERT(x, msg)
        #define LU_VERIFY(x, msg)
    #endif

}