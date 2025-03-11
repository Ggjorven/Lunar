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
    class Rc
    {
    private:
        struct Block
        {
        public:
            T* Object;
            uint32_t RefCount;

        public:
            Block(uint32_t refCount, T* object) 
                : RefCount(refCount), Object(object) 
            {
            }
        };

    private:
        // Constructors & Destructor
        template <typename... TArgs>
        explicit Rc(Block* block) 
            : m_Memory(block)
        {
        }

    public:
        Rc() 
            : m_Memory(nullptr) 
        {
        }

        Rc(std::nullptr_t n) 
            : m_Memory(nullptr)
        {
        }

        ~Rc()
        {
            if (m_Memory && --m_Memory->RefCount == 0) 
            {
                if (m_Memory->Object)
                    delete m_Memory->Object;

                delete m_Memory;
            }
        }

        // Moving
        Rc(Rc&& other) noexcept 
            : m_Memory(other.m_Memory) 
        { 
            other.m_Memory = nullptr; 
        }

        Rc& operator = (Rc&& other) noexcept
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
        Rc(Rc<T2>&& other) noexcept 
            : m_Memory(reinterpret_cast<Block*>(other.m_Memory))
        {
            other.m_Memory = nullptr;
        }

        template <typename T2>
        Rc& operator = (Rc<T2>&& other) noexcept
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
        Rc(const Rc& other) 
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

        Rc& operator = (const Rc& other)
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
        Rc(const Rc<T2>& other) 
            : m_Memory(reinterpret_cast<Block*>(other.m_Memory))
        {
            if (m_Memory) 
            {
                m_Memory->RefCount++;
            }
        }

        template <typename T2>
        Rc operator = (const Rc<T2>& other)
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

        Rc Clone() 
        { 
            return Rc(*this); 
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
        inline Rc<T2> As()
        {
            return Rc<T2>(*this);
        }

        template <typename T2>
        inline const Rc<T2> As() const
        {
            return Rc<T2>(*this);
        }

        // Static methods
        template <typename... TArgs>
        static Rc Create(TArgs&&... args)
        {
            return Rc(new Block(1, new T(std::forward<TArgs>(args)...)));
        }

        template <typename T2, typename... TArgs>
        static Rc Create(TArgs&&... args)
        {
            return Rc(new Block(1, reinterpret_cast<T*>((new T2(std::forward<TArgs>(args)...)))));
        }

    private:
        Block* m_Memory;

        template <typename T2>
        friend class Rc;
    };

}