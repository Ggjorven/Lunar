#pragma once

#include <cstdint>
#include <memory>
#include <utility>

namespace Lunar
{

    ////////////////////////////////////////////////////////////////////////////////////
	// Unique (heap) Ownership
    ////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	class Box
	{
	public:
		// Construtors & Destructor
		Box()
			: m_Object(nullptr)
		{
		}

		Box(std::nullptr_t n) 
			: m_Object(nullptr)
		{
		}

		explicit Box(T* obj)
			: m_Object(obj)
		{
		}

		template<typename T2>
		explicit Box(T2* obj)
			: m_Object(static_cast<T*>(obj))
		{
		}

		~Box()
		{
			if (m_Object)
			{
				delete m_Object;
			}
		}

		// Moving
		Box(Box&& other) noexcept
			: m_Object(other.m_Object)
		{
			other.m_Object = nullptr;
		}

		Box& operator = (Box&& other) noexcept
		{
			if (this != &other)
			{
				m_Object = other.m_Object;
				other.m_Object = nullptr;
			}

			return *this;
		}

		template<typename T2>
		Box(Box<T2>&& other) noexcept
			: m_Object(static_cast<T*>(other.m_Object))
		{
			other.m_Object = nullptr;
		}

		template<typename T2>
		Box& operator = (Box<T2>&& other) noexcept
		{
			if (this != &other)
			{
				m_Object = static_cast<T*>(other.m_Object);
				other.m_Object = nullptr;
			}

			return *this;
		}

		// Copying
		Box(const Box& other) = delete;
		Box& operator = (const Box& other) = delete;

		// Operators
		inline T* operator -> () { return Raw(); }
		inline const T* operator -> () const { return Raw(); }

		inline T& operator * () { return *Raw(); }
		inline const T& operator * () const { return *Raw(); }

		inline bool operator == (std::nullptr_t n) { return m_Object == nullptr; }
		inline bool operator != (std::nullptr_t n) { return !(*this == n); }

		// Methods
		inline void Reset()
		{ 
			if (m_Object) 
			{
                delete m_Object;
            }

			m_Object = nullptr;
		}

		inline T* Raw()
		{
			return m_Object;
		}

		inline const T* Raw() const
		{
			return m_Object;
		}

		template<typename T2>
		inline T2* RawAs()
		{
			return static_cast<T2*>(m_Object);
		}

		template<typename T2>
		inline const T2* RawAs() const
		{
			return static_cast<T2*>(m_Object);
		}

		// Static methods
		template<typename ...TArgs>
		static Box Create(TArgs&& ...args)
		{
			return Box(new T(std::forward<TArgs>(args)...));
		}

		template<typename T2, typename ...TArgs>
		static Box Create(TArgs&& ...args)
		{
			return Box<T>(new T2(std::forward<TArgs>(args)...));
		}

	private:
		T* m_Object;

		template<typename T2>
		friend class Box;
	};

}