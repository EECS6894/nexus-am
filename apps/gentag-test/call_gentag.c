#include <klib.h>

int main() {
	
	printf("Testing GENTAG\n");

	// Very simple test - don't use malloc or complex operations
	long x = 42;
	long *ptr = &x;
	long *new_ptr = ptr;

	printf("new_ptr: %p\n", new_ptr);

	asm volatile ("gentag %0, %1" : "=r" (new_ptr) : "r" (ptr));

	printf("new_ptr: %p\n", new_ptr);

	printf("GENTAG completed\n");


	printf("Testing ADDTAG\n");
	long *added_ptr = 0;
	printf("added_ptr: %p\n", added_ptr);

	asm volatile ("addtag %0, %1, 1" : "=r" (added_ptr) : "r" (new_ptr));

	printf("added_ptr: %p\n", added_ptr);

	printf("ADDTAG completed\n");


	// enable tagging in M-mode, 4-bit tags (MT_MODE=2)
	uint64_t val;
	uint64_t mt_mode_mask = 2ull << 34;

	asm volatile ("csrrs %0, 0x30A, %1" : "=r"(val) : "r"(mt_mode_mask));  // menvcfg
	printf("menvcfg=%p\n", val);

	asm volatile ("csrrc %0, 0x30A, %1" : "=r"(val) : "r"(mt_mode_mask));  // menvcfg
	printf("menvcfg=%p\n", val);

	return 0;
}
