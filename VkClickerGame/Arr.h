#pragma once
#include "mem.h"
#include "Link.h"

namespace mem
{
	template <typename T>
	struct Arr
	{
		Arr();
		Arr(uint8_t arena, uint32_t length);
		Arr(T* ptr, uint32_t i);
		T& operator[](uint32_t i) const;
		uint32_t length() const;
		T* ptr() const;

		Arr<T> copy(uint8_t arena, uint32_t from = 0, int32_t to = -1) const;
		void reverse() const;
		void put(uint32_t i, Arr& arr) const;
		void put(uint32_t i, T* ptr, uint32_t length) const;
		void point(T* ptr, uint32_t length = 0);

		template <typename U>
		void iter(U func, bool reverse = false) const;
		template <typename U>
		bool iterb(U func, bool reverse = false) const;
		template <typename U>
		Arr<T> get(uint8_t arena, U func) const;
		template <typename U>
		void sort(U func) const;

		static Arr<T> combine(uint8_t arena, Arr<T>& a, Arr<T>& b);

	protected:
		T* _ptr = nullptr;
		uint32_t _length = 0;
	};

	template<typename T>
	inline Arr<T>::Arr()
	{
	}

	template<typename T>
	inline Arr<T>::Arr(uint8_t arena, uint32_t length)
	{
		_ptr = mem::alloc<T>(arena, length);
		_length = length;
	}
	template<typename T>
	inline Arr<T>::Arr(T* ptr, uint32_t i)
	{
		point(ptr, i);
	}
	template<typename T>
	inline T& Arr<T>::operator[](uint32_t i) const
	{
		assert(i < _length);
		return _ptr[i];
	}
	template<typename T>
	inline uint32_t Arr<T>::length() const
	{
		return _length;
	}
	template<typename T>
	inline T* Arr<T>::ptr() const
	{
		return _ptr;
	}
	template<typename T>
	inline Arr<T> Arr<T>::copy(uint8_t arena, uint32_t from, int32_t to) const
	{
		assert(from >= 0);
		assert(to < (int32_t)_length);
		assert(to > -(int32_t)_length);
		uint32_t uto = to >= 0 ? to : _length - abs(to + 1);
		uint32_t l = uto - from;
		auto oArr = Arr<T>(arena, l);
		memcpy(oArr.ptr(), &_ptr[from], sizeof(T) * l);
		return oArr;
	}
	template<typename T>
	inline void Arr<T>::reverse() const
	{
		uint32_t to = _length / 2;
		for (uint32_t i = 0; i < to; i++)
		{
			auto temp = _ptr[i];
			uint32_t ind = _length - 1 - i;
			_ptr[i] = _ptr[ind];
			_ptr[ind] = temp;
		}
	}
	template<typename T>
	inline void Arr<T>::put(uint32_t i, Arr& arr) const
	{
		put(i, arr._ptr, arr._length);
	}
	template<typename T>
	inline void Arr<T>::put(uint32_t i, T* ptr, uint32_t length) const
	{
		memcpy(&_ptr[i], ptr, sizeof(T) * length);
	}
	template<typename T>
	inline void Arr<T>::point(T* ptr, uint32_t length)
	{
		_ptr = ptr;
		_length = length;
	}
	template<typename T>
	inline Arr<T> Arr<T>::combine(uint8_t arena, Arr<T>& a, Arr<T>& b)
	{
		auto arr = Arr<T>(arena, a.length() + b.length());
		memcpy(arr._ptr, a._ptr, a._length * sizeof(T));
		memcpy(&arr._ptr[a._length], b._ptr, b._length * sizeof(T));
		return arr;
	}
	template<typename T>
	template<typename U>
	inline void Arr<T>::iter(U func, bool reverse) const
	{
		if (reverse)
		{
			for (int32_t i = _length - 1; i >= 0; i--)
				func(_ptr[i], i);
			return;
		}
			
		for (uint32_t i = 0; i < _length; i++)
			func(_ptr[i], i);
	}
	template<typename T>
	template<typename U>
	inline bool Arr<T>::iterb(U func, bool reverse) const
	{
		if (reverse)
		{
			for (int32_t i = _length - 1; i >= 0; i--)
				if (!func(_ptr[i], i))
					return false;
			return true;
		}

		for (uint32_t i = 0; i < _length; i++)
			if (!func(_ptr[i], i))
				return false;
		return true;
	}
	template<typename T>
	template<typename U>
	inline Arr<T> Arr<T>::get(uint8_t arena, U func) const
	{
		auto _ = arena == FRAM ? mem::manualScope(FRAM) : mem::scope(FRAM);
		auto link = Link<T>();
		for (uint32_t i = 0; i < _length; i++)
			if (func(_ptr[i], i))
				link.add(FRAM) = _ptr[i];
		return link.arr(arena);
	}
	template<typename T>
	template<typename U>
	inline void Arr<T>::sort(U func) const
	{
		for (size_t i = 1; i < _length; ++i)
		{
			size_t idx = i;
			while (idx > 0)
			{
				auto& current = _ptr[idx];
				auto& other = _ptr[idx - 1];

				if (!func(current, other))
					break;

				T temp = current;
				current = other;
				other = temp;

				--idx;
			}
		}
	}
}
