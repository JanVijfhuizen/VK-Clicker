#pragma once
#include "Arr.h"
#include "Math.h"

namespace mem 
{
	template <typename T>
	struct Queue final : Arr<T>
	{
		Queue(uint8_t arena, uint32_t length);

		T& add();
		[[nodiscard]] T& peek() const;
		T pop();
		void clear();

	private:
		uint32_t _count = 0;
		uint32_t _front = 0;

		uint32_t getIndex(uint32_t i) const;
	};
	template<typename T>
	inline Queue<T>::Queue(uint8_t arena, uint32_t length) : Arr<T>(arena, length)
	{
		
	}
	template<typename T>
	inline T& Queue<T>::add()
	{
		auto& ret = Arr<T>::_ptr[getIndex(_count++)];
		if (_count > Arr<T>::_length)
			_front = (_front + 1) % Arr<T>::_length;
		_count = jv::Min<uint32_t>(_count, Arr<T>::_length);
		return ret;
	}
	template<typename T>
	inline T& Queue<T>::peek() const
	{
		assert(_count > 0);
		return Arr<T>::_ptr[getIndex(0)];
	}
	template<typename T>
	inline T Queue<T>::pop()
	{
		assert(_count > 0);
		auto t = Arr<T>::_ptr[getIndex(0)];
		_front = (_front + 1) % Arr<T>::_length;
		--_count;
		return t;
	}
	template<typename T>
	inline void Queue<T>::clear()
	{
		_front = 0;
		_count = 0;
	}
	template<typename T>
	inline uint32_t Queue<T>::getIndex(uint32_t i) const
	{
		return (i + _front + Arr<T>::_length) % Arr<T>::_length;
	}
}


