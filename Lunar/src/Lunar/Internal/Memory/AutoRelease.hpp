#pragma once

#include <cstdint>
#include <memory>
#include <ranges>
#include <utility>
#include <functional>

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // AutoRelease
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename T, T Invalid = nullptr>
    class AutoRelease
    {
    public:
        // Constructors & Destructor
        AutoRelease()
            : AutoRelease({}, nullptr)
        {
        }

        explicit AutoRelease(T obj, const std::function<void(T object)> deleter = std::default_delete<T>())
            : m_Object(obj), m_Deleter(deleter)
        {
        }

        ~AutoRelease()
        {
            if (m_Object != Invalid && (bool)m_Deleter)
            {
                m_Deleter(m_Object);
            }
        }

        // Copying
        AutoRelease(const AutoRelease& other) = delete;
        AutoRelease& operator = (const AutoRelease& other) = delete;

        // Moving / Swapping
        AutoRelease(AutoRelease&& other)
        {
            Swap(other);
        }

        AutoRelease& operator = (AutoRelease&& other) noexcept
        {
            auto newObj = AutoRelease(std::move(other));
            Swap(newObj);

            return *this;
        }

        void Swap(AutoRelease& other) noexcept
        {
            std::ranges::swap(m_Object, other.m_Object);
            std::ranges::swap(m_Deleter, other.m_Deleter);
        }

        // Retrieval
        T Get() const
        {
            return m_Object;
        }

        operator T() const
        {
            return Get();
        }
        
    private:
        T m_Object;
        std::function<void(T object)> m_Deleter;
    };

}