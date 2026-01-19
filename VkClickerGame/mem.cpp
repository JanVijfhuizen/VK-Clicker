#include "pch.h"
#include "mem.h"
#include "Arena.h"

namespace mem
{
	jv::Arena* arenas = nullptr;
	uint32_t arenasLength;

	struct PScope final {
		PScope() {}
		PScope(Scope& scope, ARENA arena, bool manual) {
			scope._arena = arena;
			scope._manual = manual;
			scope._scope = arenas[arena].CreateScope();
		}
	};

	void* MAlloc(const uint32_t size)
	{
		return malloc(size);
	}

	void MFree(void* ptr)
	{
		return free(ptr);
	}

	void init(const Info& info)
	{
		assert(!arenas);
		uint32_t len = 2 + (info.persistentLength == 0 ? 1 : info.persistentLength);
		arenas = reinterpret_cast<jv::Arena*>(malloc(sizeof(jv::Arena) * len));

		jv::ArenaCreateInfo aInfo{};
		aInfo.alloc = MAlloc;
		aInfo.free = MFree;

		aInfo.memorySize = info.tempSize;
		arenas[TEMP] = jv::Arena::Create(aInfo);
		aInfo.memorySize = info.frameSize;
		arenas[FRAM] = jv::Arena::Create(aInfo);
		arenasLength = len;

		for (uint32_t i = 2; i < len; i++)
		{
			aInfo.memorySize = info.persistentInitSizes ? info.persistentInitSizes[i] : info.persistentDefaultSize;
			arenas[i] = jv::Arena::Create(aInfo);
		}
	}
	void end()
	{
		assert(arenas);
		for (uint32_t i = 0; i < arenasLength; i++)
			jv::Arena::Destroy(arenas[i]);
		free(arenas);
		arenas = nullptr;
	}
	bool active()
	{
		return arenas;
	}
	Scope scope(ARENA arena)
	{
		Scope scope{};
		PScope(scope, arena, false);
		return scope;
	}
	Scope manualScope(ARENA arena)
	{
		Scope scope{};
		PScope(scope, arena, true);
		return scope;
	}
	void* manualAlloc(ARENA arena, size_t size)
	{
		return arenas[arena].Alloc(size);
	}
	void frame()
	{
#ifdef _DEBUG
		jv::Arena* arena = &arenas[TEMP];
		do
		{
			assert(arena->front == 0);
			arena = arena->next;
		} while (arena);
#endif // _DEBUG

		arenas[FRAM].Clear();
	}
	void Scope::clear()
	{
		arenas[_arena].DestroyScope(_scope);
	}
	Scope::~Scope()
	{
		if (_manual)
			return;
		clear();
	}
}