#pragma once

#include <cstdint>
#include <string_view>

namespace Lunar
{

	class Hash
	{
    public:
        // Hash functions
        static constexpr size_t fnv1a(const std::string_view str)
        {
            constexpr size_t fnvPrime = 1099511628211u;
            constexpr size_t offsetBasis = 14695981039346656037u;

            size_t hash = offsetBasis;
            for (char c : str) 
            {
                hash ^= static_cast<size_t>(c);
                hash *= fnvPrime;
            }

            return hash;
        }

    public:
        // Helper functions
        static constexpr size_t Combine(const size_t hash1, const size_t hash2)
        {
            size_t combined = hash1 + 0x9e3779b97f4a7c15;
            combined = (combined ^ (hash2 >> 30)) * 0xbf58476d1ce4e5b9;
            combined = (combined ^ (combined >> 27)) * 0x94d049bb133111eb;

            return combined ^ (combined >> 31);
        }
	};
}