#include "pch.h"
#include "Str.h"

namespace mem
{
	Str::Str()
	{
	}
	Str::Str(uint8_t arena, uint32_t length) : Arr<char>(arena, length)
	{
	}
	Str::Str(uint8_t arena, const char* string) : Arr<char>(arena, strlen(string))
	{
		memcpy(Arr<char>::_ptr, string, length());
	}
	void Str::_put(uint32_t i, const char* str)
	{
		memcpy(&_ptr[i], str, strlen(str));
	}
	uint32_t Str::_getLength(const char* str)
	{
		return strlen(str);
	}
}
