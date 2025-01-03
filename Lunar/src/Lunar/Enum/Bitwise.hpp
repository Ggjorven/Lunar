#pragma once

#include <cstdint>
#include <type_traits>

namespace Lunar::Enum::Internal
{

    namespace EnumBitmask
    {
        template <typename Enum>
        struct EnumWrapper
        {
        public:
            constexpr explicit operator bool() const noexcept { return (e != Enum{ 0 }); }

            constexpr operator Enum() const noexcept { return e; }

        public:
            Enum e;
        };

        template <typename Enum>
        EnumWrapper(Enum e) -> EnumWrapper<Enum>;
    }

    #define LU_ENABLE_BITWISE(e)                                                                                            \
        static_assert(std::is_enum_v<e>, "e must be an enum type");                                                         \
        [[nodiscard]]                                                                                                       \
        inline constexpr auto operator&(e lhs, e rhs) noexcept                                                              \
        {                                                                                                                   \
            using U = std::underlying_type_t<e>;                                                                            \
            return ::Lunar::Enum::Internal::EnumBitmask::EnumWrapper{ e(static_cast<U>(lhs) & static_cast<U>(rhs)) };       \
        }                                                                                                                   \
        [[nodiscard]]                                                                                                       \
        inline constexpr auto operator|(e lhs, e rhs) noexcept                                                              \
        {                                                                                                                   \
            using U = std::underlying_type_t<e>;                                                                            \
            return ::Lunar::Enum::Internal::EnumBitmask::EnumWrapper{ e(static_cast<U>(lhs) | static_cast<U>(rhs)) };       \
        }                                                                                                                   \
        [[nodiscard]]                                                                                                       \
        inline constexpr auto operator^(e lhs, e rhs) noexcept                                                              \
        {                                                                                                                   \
            using U = std::underlying_type_t<e>;                                                                            \
            return ::Lunar::Enum::Internal::EnumBitmask::EnumWrapper{ e(static_cast<U>(lhs) ^ static_cast<U>(rhs)) };       \
        }                                                                                                                   \
        [[nodiscard]]                                                                                                       \
        inline constexpr e operator~(e value) noexcept                                                                      \
        {                                                                                                                   \
            using U = std::underlying_type_t<e>;                                                                            \
            return e(~static_cast<U>(value));                                                                               \
        }                                                                                                                   \
        inline constexpr e& operator&=(e& lhs, e rhs) noexcept                                                              \
        {                                                                                                                   \
            return lhs = (lhs & rhs);                                                                                       \
        }                                                                                                                   \
        inline constexpr e& operator|=(e& lhs, e rhs) noexcept                                                              \
        {                                                                                                                   \
            return lhs = (lhs | rhs);                                                                                       \
        }                                                                                                                   \
        inline constexpr e& operator^=(e& lhs, e rhs) noexcept                                                              \
        {                                                                                                                   \
            return lhs = (lhs ^ rhs);                                                                                       \
        }

}