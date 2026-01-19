#pragma once
#include "Arr.h"
#include "KeyPair.h"

namespace mem 
{
	template <typename T>
	struct Map final : Arr<KeyPair<T>>
	{
		Map(uint8_t arena, uint32_t length);
		T& operator[](uint64_t key) const;
		T& insert(uint64_t key);
		[[nodiscard]] T* contains(uint64_t key) const;
		void erase(T& value);
		uint32_t count();

	private:
		uint32_t _count = 0;

		[[nodiscard]] uint64_t _hash(uint64_t key) const;
	};
	template<typename T>
	inline Map<T>::Map(uint8_t arena, uint32_t length) : Arr<KeyPair<T>>(arena, length)
	{
	}
	template<typename T>
	inline T& Map<T>::operator[](uint64_t key) const
	{
		return *contains(key);
	}
	template<typename T>
	inline T& Map<T>::insert(uint64_t key)
	{
		assert(_count < Arr<KeyPair<T>>::_length);

		// If it already contains this value, replace the old one with the newer value.
		if (auto value = contains(key))
			return *value;

		const uint64_t hash = _hash(key);

		for (uint64_t i = 0; i < Arr<KeyPair<T>>::_length; ++i)
		{
			const uint64_t index = (hash + i) % Arr<KeyPair<T>>::_length;
			auto& keyPair = Arr<KeyPair<T>>::_ptr[index];
			// Set to true the first time the key group has been found.
			if (keyPair.key != SIZE_MAX)
				continue;

			keyPair.key = key;
			++_count;

			return keyPair.value;
		}
		// Should not get here.
		assert(false);
		T t;
		return t;
	}
	template<typename T>
	inline T* Map<T>::contains(uint64_t key) const
	{
		assert(_count <= Arr<KeyPair<T>>::_length);

		// Get and use the hash as an index.
		const uint64_t hash = _hash(key);

		for (uint64_t i = 0; i < Arr<KeyPair<T>>::_length; ++i)
		{
			const uint64_t index = (hash + i) % Arr<KeyPair<T>>::_length;
			auto& keyPair = Arr<KeyPair<T>>::_ptr[index];

			// If the hash is different, continue.
			if (keyPair.key == key)
				return &keyPair.value;
		}

		return nullptr;
	}
	template<typename T>
	inline void Map<T>::erase(T& value)
	{
		uint64_t index;
		const bool contains = contains(value, index);
		assert(contains);
		assert(_count > 0);

		auto& keyPair = Arr<KeyPair<T>>::_ptr[index];

		// Check how big the key group is.
		uint64_t i = 1;
		while (i < Arr<KeyPair<T>>::_length)
		{
			const uint64_t otherIndex = (index + i) % Arr<KeyPair<T>>::_length;
			auto& otherKeyPair = Arr<KeyPair<T>>::_ptr[otherIndex];
			if (otherKeyPair.key != keyPair.key)
				break;
			++i;
		}

		// Setting the key pair value to the default value.
		keyPair = {};
		// Move the key group one place backwards by swapping the first and last index.
		T t = Arr<KeyPair<T>>::_ptr[index];
		Arr<KeyPair<T>>::_ptr[index] = Arr<KeyPair<T>>::_ptr[index + i - 1];
		Arr<KeyPair<T>>::_ptr[index + i - 1] = t;
		--_count;
	}
	template<typename T>
	inline uint32_t Map<T>::count()
	{
		return _count;
	}
	template<typename T>
	inline uint64_t Map<T>::_hash(uint64_t key) const
	{
		return key % Arr<KeyPair<T>>::_length;
	}
}

