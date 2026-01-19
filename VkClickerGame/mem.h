#pragma once

enum {
	TEMP,
	FRAM,
	PERS,
	NONE = 255
};

#define ARENA uint8_t
#define PERN(i) PERS + i
#define RPERN(i) i - PERS

namespace mem
{
	struct IScoped {
		friend struct Scope;
	protected:
		virtual void OnScopeClear() = 0;
	private:
		IScoped* _next = nullptr;
	};

	struct Scope final {
		friend class PScope;
		void clear();
		~Scope();
		bool operator==(const Scope& other);
		void bind(IScoped& scoped);
	private:
		uint64_t _scope;
		ARENA _arena;
		bool _manual;
		IScoped* _scoped = nullptr;
	};

	struct Info final {
		uint32_t* persistentInitSizes = nullptr;
		uint32_t persistentLength = 0;
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

