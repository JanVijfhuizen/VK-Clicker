#pragma once

#define ARENA uint8_t

enum {
	TEMP,
	FRAM,
	PERS,
	NONE = 255
};

namespace mem
{
	struct Scope final {
		friend class PScope;
		void clear();
		~Scope();
	private:
		uint64_t _scope;
		ARENA _arena;
		bool _manual;
	};

	struct Info final {
		uint32_t* persistentInitSizes = nullptr;
		uint32_t persistentLength = 1;
		uint32_t persistentDefaultSize = 4096 * 256 * 32;
		uint32_t tempSize = 4096 * 256 * 32;
		uint32_t frameSize = 4096 * 256;
	};

	void init(const Info& info = {});
	void end();
	bool active();

	Scope scope(ARENA arena);
	Scope manualScope(ARENA arena);
	void* manualAlloc(ARENA arena, size_t size);
	template <typename T>
	T* alloc(ARENA arena, uint32_t count = 1);
	void frame();

	template<typename T>
	T* alloc(ARENA arena, uint32_t count)
	{
		void* ptr = manualAlloc(arena, sizeof(T) * count);
		T* ptrType = static_cast<T*>(ptr);
		for (uint32_t i = 0; i < count; ++i)
			new(&ptrType[i]) T();
		return ptrType;
	}
}

