#pragma once
#include "Arr.h"

namespace mem
{
	struct Str final : Arr<char>
	{
		Str();
		Str(uint8_t arena, uint32_t length);
		Str(uint8_t arena, const char* string);

		void set(const char* string);
		template <typename ...Args>
		static Str f(uint8_t arena, Args... args);
		static Str i(uint8_t arena, int32_t i);
	private:
		template <typename ...Args>
		static uint32_t _getLength(const char* str, Args... args);
		static uint32_t _getLength(const char* str);

		template <typename ...Args>
		void _put(uint32_t i, const char* str, Args... args);
		void _put(uint32_t i, const char* str);
	};
	template<typename ...Args>
	inline Str Str::f(uint8_t arena, Args ...args)
	{
		Str str = Str(arena, _getLength(args...) + 1);
		str._put(0, args...);
		str._ptr[str._length - 1] = '\0';
		return str;
	}
	template<typename ...Args>
	inline uint32_t Str::_getLength(const char* str, Args ...args)
	{
		return strlen(str) + _getLength(args...);
	}
	template<typename ...Args>
	inline void Str::_put(uint32_t i, const char* str, Args ...args)
	{
		const uint32_t len = strlen(str);
		memcpy(&_ptr[i], str, len);
		_put(i + len, args...);
	}
}


