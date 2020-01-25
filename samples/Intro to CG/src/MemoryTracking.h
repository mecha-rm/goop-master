#pragma once
#include <cstdlib>
#include <new>
#include <cstdint>
#include <string>

class MemoryTracking {
public:
	static size_t NumAllocs;
	static size_t TotalAllocs;
	static size_t NumBytes;
	static size_t TotalBytes;
	
	static void NotifyAlloc(size_t size, void* block);
	static void NotifyDealloc(size_t size, void* block);

	static void PushStack(const std::string_view& name);
	static void PopStack(size_t* numAllocs, size_t* numBytes);	

private:
	struct StackEntry {
		char Name[100];
		size_t NumAllocs;
		size_t NumBytes;
	};
	static StackEntry myStack[100];
	static uint32_t   myStackIx;
};

inline size_t get_block_size(void* block) {
	return *(((size_t*)block) - 1);
}

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(_Size) _VCRT_ALLOCATOR
void* operator new(size_t _Size);
void operator delete(void* block);