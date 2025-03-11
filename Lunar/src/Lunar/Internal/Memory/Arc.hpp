#pragma once

#include <cstdint>
#include <utility>
#include <atomic>

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Atomic Ref Counting
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    class Arc
    {
    private:
        struct Block
        {
        public:
            T* Object;
            std::atomic<uint32_t> RefCount;

        public:
            Block(uint32_t refCount, T* object) 
                : RefCount(refCount), Object(object) 
            {
            }
        };

    private:
        // Constructors & Destructor
        template <typename... TArgs>
        explicit Arc(Block* block) 
            : m_Memory(block)
        {
        }

    public:
        Arc() 
            : m_Memory(nullptr) 
        {
        }

        Arc(std::nullptr_t) 
            : m_Memory(nullptr)
        {
        }

        ~Arc()
        {
            if (m_Memory && --m_Memory->RefCount == 0) 
            {
                if (m_Memory->Object)
                    delete m_Memory->Object;

                delete m_Memory;
            }
        }

        // Moving
        Arc(Arc&& other) noexcept 
            : m_Memory(other.m_Memory) 
        { 
            other.m_Memory = nullptr; 
        }

        Arc& operator = (Arc&& other) noexcept
        {
            if (this != &other) 
            {
                // Remove a reference from current object
                if (m_Memory && --m_Memory->RefCount == 0) 
                {
                    if (m_Memory->Object)
                        delete m_Memory->Object;

                    delete m_Memory;
                }

                // Set new objects
                m_Memory = other.m_Memory;
                other.m_Memory = nullptr;
            }

            return *this;
        }

        template <typename T2>
        Arc(Arc<T2>&& other) noexcept 
            : m_Memory(reinterpret_cast<Block*>(other.m_Memory))
        {
            other.m_Memory = nullptr;
        }

        template <typename T2>
        Arc& operator = (Arc<T2>&& other) noexcept
        {
            if (this != &other) 
            {
                // Remove a reference from current object
                if (m_Memory && --m_Memory->RefCount == 0) 
                {
                    if (m_Memory->Object)
                        delete m_Memory->Object;

                    delete m_Memory;
                }

                // Set new objects
                m_Memory = reinterpret_cast<Block*>(other.m_Memory);
                other.m_Memory = nullptr;
            }

            return *this;
        }

        // Copying
        Arc(const Arc& other) 
            : m_Memory(nullptr)
        {
            if (this != &other) 
            {
                // Remove a reference from current object
                if (m_Memory && --m_Memory->RefCount == 0) 
                {
                    if (m_Memory->Object)
                        delete m_Memory->Object;

                    delete m_Memory;
                }

                // Set new objects & increase reference count
                m_Memory = other.m_Memory;
                if (m_Memory) 
                {
                    m_Memory->RefCount++;
                }
            }
        }

        Arc& operator = (const Arc& other)
        {
            if (this != &other) 
            {
                // Remove a reference from current object
                if (m_Memory && --m_Memory->RefCount == 0) 
                {
                    if (m_Memory->Object)
                        delete m_Memory->Object;

                    delete m_Memory;
                }

                // Set new objects & increase reference count
                m_Memory = other.m_Memory;
                if (m_Memory) 
                {
                    m_Memory->RefCount++;
                }
            }

            return *this;
        }

        template <typename T2>
        Arc(const Arc<T2>& other) 
            : m_Memory(reinterpret_cast<Block*>(other.m_Memory))
        {
            if (m_Memory) 
            {
                m_Memory->RefCount++;
            }
        }

        template <typename T2>
        Arc operator = (const Arc<T2>& other)
        {
            if (this != &other) 
            {
                // Remove a reference from current object
                if (m_Memory && --m_Memory->RefCount == 0) 
                {
                    if (m_Memory->Object)
                        delete m_Memory->Object;

                    delete m_Memory;
                }

                // Set new objects & increase reference count
                m_Memory = reinterpret_cast<Block*>(other.m_Memory);
                if (m_Memory) 
                {
                    m_Memory->RefCount++;
                }
            }
        }

        Arc Clone() 
        { 
            return Arc(*this); 
        }

        // Operators
        inline T* operator -> () { return Raw(); }
        inline const T* operator -> () const { return Raw(); }

        inline T& operator * () { return *Raw(); }
        inline const T& operator * () const { return *Raw(); }

        inline operator bool() const { return m_Memory != nullptr; }

        inline bool operator == (std::nullptr_t n) { return m_Memory == nullptr; }
        inline bool operator != (std::nullptr_t n) { return !(*this == n); }

        // Methods
        inline uint32_t RefCount() const { return m_Memory ? m_Memory->RefCount.load(std::memory_order_acquire) : 0; }

        inline void Reset() // Note: Make sure what you're doing when you call this.
        {
            if (m_Memory && m_Memory->Object)
            {
                delete m_Memory->Object;
                m_Memory->Object = nullptr;
            }
        }

        inline T* Raw()
        {
            if (m_Memory) 
            {
                return m_Memory->Object;
            }

            return nullptr;
        }

        inline const T* Raw() const
        {
            if (m_Memory) 
            {
                return m_Memory->Object;
            }

            return nullptr;
        }

        template <typename T2>
        inline T2* RawAs()
        {
            if (m_Memory) 
            {
                return reinterpret_cast<T2*>(m_Memory->Object);
            }

            return nullptr;
        }

        template <typename T2>
        inline const T2* RawAs() const
        {
            if (m_Memory) 
            {
                return reinterpret_cast<T2*>(m_Memory->Object);
            }

            return nullptr;
        }

        template <typename T2>
        inline Arc<T2> As()
        {
            return Arc<T2>(*this);
        }

        template <typename T2>
        inline const Arc<T2> As() const
        {
            return Arc<T2>(*this);
        }

        // Static methods
        template <typename... TArgs>
        static Arc Create(TArgs&&... args)
        {
            return Arc(new Block(1, new T(std::forward<TArgs>(args)...)));
        }

        template <typename T2, typename... TArgs>
        static Arc Create(TArgs&&... args)
        {
            return Arc(new Block(1, reinterpret_cast<T*>((new T2(std::forward<TArgs>(args)...)))));
        }

    private:
        Block* m_Memory;

        template <typename T2>
        friend class Arc;
    };

}