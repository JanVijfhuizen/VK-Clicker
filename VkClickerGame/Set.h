#pragma once
#include <cstdint>
#include "Vec.h"

namespace mem
{
	template <typename T>
	struct Set final {
		Set(uint8_t arena, uint32_t length);
		void insert(uint64_t key, T& value);
		bool contains(uint64_t key, T& value);
		Vec<T> vec();
	private:
		Arr<uint32_t> _set;
		Vec<T> _values;
	};

	template<typename T>
	inline Set<T>::Set(uint8_t arena, uint32_t length)
	{
		_set = Arr<uint32_t>(arena, length);
		_set.set(-1);
		_values = Vec<T>(arena, length);
	}
	template<typename T>
	inline void Set<T>::insert(uint64_t key, T& value)
	{
		if (contains(key, value))
			return;
		_set[key] = true;
		_values.add() = value;
	}
	template<typename T>
	inline bool Set<T>::contains(uint64_t key, T& value)
	{
		return _set[key] != -1;
	}
	template<typename T>
	inline Vec<T> Set<T>::vec()
	{
		return _values;
	}
}
