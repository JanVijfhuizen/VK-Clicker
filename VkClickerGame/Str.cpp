#include "pch.h"
#include "Str.h"
#include <sstream>

namespace mem
{
	Str::Str()
	{
	}
	Str::Str(uint8_t arena, uint32_t length) : Arr<char>(arena, length)
	{
	}
	Str::Str(uint8_t arena, const char* string) : Arr<char>(arena, strlen(string) + 1)
	{
		set(string);
	}
	void Str::set(const char* string)
	{
		memcpy(Arr<char>::_ptr, string, length());
		Arr<char>::_ptr[strlen(string)] = '\0';
	}
	Str Str::i(uint8_t arena, int32_t i)
	{
		std::stringstream strs;
		strs << i;
		std::string temp = strs.str();
		char* char_type = (char*)temp.c_str();
		return Str(arena, char_type);
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
