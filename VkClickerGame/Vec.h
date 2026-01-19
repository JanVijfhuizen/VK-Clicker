#pragma once
#include "Arr.h"

namespace mem
{
	template <typename T>
	struct Vec : Arr<T>
	{
		Vec();
		Vec(uint8_t arena, uint32_t length);
		Vec(Arr<T>& arr);
		T& add();
		void clear();
		uint32_t count();
		Arr<T> arr();
		void setCount(uint32_t i);
	private:
		uint32_t _count = 0;
	};
	template<typename T>
	inline Vec<T>::Vec()
	{
	}
	template<typename T>
	inline Vec<T>::Vec(uint8_t arena, uint32_t length) : Arr<T>(arena, length)
	{
	}
	template<typename T>
	inline Vec<T>::Vec(Arr<T>& arr)
	{
		Arr<T>::_ptr = arr.ptr();
		Arr<T>::_length = arr.length();
		_count = arr.length();
	}
	template<typename T>
	inline T& Vec<T>::add()
	{
		assert(_count < Arr<T>::_length);
		return Arr<T>::_ptr[_count++];
	}
	template<typename T>
	inline void Vec<T>::clear()
	{
		_count = 0;
	}
	template<typename T>
	inline uint32_t Vec<T>::count()
	{
		return _count;
	}
	template<typename T>
	inline Arr<T> Vec<T>::arr()
	{
		return Arr<T>(Arr<T>::_ptr, _count);
	}
	template<typename T>
	inline void Vec<T>::setCount(uint32_t i)
	{
		assert(i <= Arr<T>::_length);
		_count = i;
	}
}

