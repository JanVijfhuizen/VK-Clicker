#pragma once
#include "mem.h"

namespace mem
{
	template <typename T>
	struct Arr;

	template <typename T>
	struct Link final
	{
		T& add(uint8_t arena);
		uint32_t length() const;
		Arr<T> arr(uint8_t arena) const;

	private:
		struct Node final
		{
			T value;
			Node* next;
		};

		Node* _next = nullptr;
	};
	template<typename T>
	inline T& Link<T>::add(uint8_t arena)
	{
		auto ptr = mem::alloc<Node>(arena);
		ptr->value = {};
		ptr->next = _next;
		_next = ptr;
		return ptr->value;
	}
	template<typename T>
	inline uint32_t Link<T>::length() const
	{
		auto next = _next;
		uint32_t n = 0;
		while (next)
		{
			++n;
			next = next->next;
		}
		return n;
	}
	template<typename T>
	inline Arr<T> Link<T>::arr(uint8_t arena) const
	{
		auto arr = Arr<T>(arena, length());
		auto next = _next;
		uint32_t i = 0;
		while (next)
		{
			arr[i++] = next->value;
			next = next->next;
		}
		return arr;
	}
}