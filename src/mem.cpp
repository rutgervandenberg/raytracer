#include <cstdlib>
#include "mem.hpp"

// allocate on 16 byte boundary
void* ialloc(int size) {
	char* ptr = (char*)malloc(size + 16 + 16);
	char* data = (ptr + 16 + 16 - (long long)ptr % 16);
	char* info = (char*)data - 16; // 16 bytes in size
	*(char**)info = ptr;
	return data;
}

// free on 16 byte boundary
void ifree(void* ptr) {
	void* real = *(void**)((char*)ptr - 16);
	free(real);
}