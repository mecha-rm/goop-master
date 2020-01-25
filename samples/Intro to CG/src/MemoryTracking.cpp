#include "MemoryTracking.h"
#include "Logging.h"

size_t MemoryTracking::NumAllocs = 0;
size_t MemoryTracking::TotalAllocs = 0;
size_t MemoryTracking::NumBytes = 0;
size_t MemoryTracking::TotalBytes = 0;

MemoryTracking::StackEntry MemoryTracking::myStack[100];
uint32_t   MemoryTracking::myStackIx;

void MemoryTracking::NotifyAlloc(size_t size, void* block) {
	myStack[myStackIx].NumAllocs++;
	myStack[myStackIx].NumBytes += size;
	NumAllocs++;
	NumBytes += size;
	TotalAllocs++;
	TotalBytes += size;
}

void MemoryTracking::NotifyDealloc(size_t size, void* block) {
	myStack[myStackIx].NumAllocs--;
	myStack[myStackIx].NumBytes -= size;
	NumAllocs--;
	NumBytes -= size;
}

void MemoryTracking::PushStack(const std::string_view& name) {
	myStackIx++;
	myStack[myStackIx].NumBytes = 0;
	myStack[myStackIx].NumAllocs = 0;
	name.copy(myStack[myStackIx].Name, 0, 100 < name.size() ? name.size() : 100);
}

void MemoryTracking::PopStack(size_t* numAllocs, size_t* numBytes) {
	if (numAllocs)
		* numAllocs = myStack[myStackIx].NumAllocs;
	if (numBytes)
		* numBytes = myStack[myStackIx].NumBytes;
	myStackIx--;
}

void* operator new(size_t size) {
	void* result = malloc(size);
	MemoryTracking::NotifyAlloc(size, result);
	return result;
}

void operator delete(void* block) {
	if (block) {
		size_t blockSize = _msize(block);
		MemoryTracking::NotifyDealloc(blockSize, block);
		free(block);
	}
}
