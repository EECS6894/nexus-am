#include <klib.h>

int main() {

	printf("Displaying Priviledge Mode");
	uint64_t mstatus;
	
	asm volatile ("csrr %0, mstatus" : "=r"(mstatus));
	
	uint64_t mpp = (mstatus >> 11) & 0x3;  // bits 12:11
	
	printf("\tmstatus = 0x%lx\n", mstatus);
	printf("\tMPP = %llu (", mpp);
	
	if (mpp == 3) printf("\tM-mode");
	else if (mpp == 1) printf("\tS-mode");
	else if (mpp == 0) printf("\tU-mode");
	else printf("\tUnknown");
	printf(")\n");
	
	printf("Testing GENTAG\n");

	// Very simple test - don't use malloc or complex operations
	long x = 42;
	long *gentag_ptr = &x;
	long *addtag_ptr = 0;

	printf("\tgentag_ptr: %p\n", genttag_ptr);

	printf("\texecuting \"gentag gentag_ptr, zero\"\n");
	asm volatile ("\tgentag %0, zero" : "=r" (gentag_ptr));

	printf("\tgentag_ptr: %p\n", genttag_ptr);

	printf("Testing ADDTAG\n");
	printf("\taddtag_ptr: %p\n", addtag_ptr);
	
	printf("\texecuting \"addtag addtag_ptr, gentag_ptr, 1\"\n");
	asm volatile ("\taddtag %0, %1, 1" : "=r" (addtag_ptr) : "r" (gentag_ptr));
	
	printf("\taddtag_ptr: %p\n", addtag_ptr);
	

	printf("Setting Virtural Address Translation Mode\n");
	uint64_t satp;
	uint64_t mode;
	
	asm volatile ("csrr %0, satp" : "=r"(satp));
	printf("\treading: satp = 0x%lx\n", satp);
	satp |= (0x9ULL<<60); // make upper 4 bits = 9
	satp |=  0x80100ULL; // set the ptbr
	printf("\twriting: satp = 0x%lx\n", satp);
	asm volatile ("csrw satp, %0" :: "r"(satp));

	asm volatile ("csrr %0, satp" : "=r"(satp));
	printf("\treading: satp = 0x%lx\n", satp);

	// Extract the MODE field (bits 63:60)
	mode = satp >> 60;
	printf("\tTranslation mode: %llu\n", mode);

	if (mode == 0) {
		printf("\tNO ADDRESS TRANSLATION (bare)\n");
	} else if (mode == 8) {
		printf("\tSv39 address translation ENABLED\n");
	} else if (mode == 9) {
		printf("\tSv48 address translation ENABLED\n");
	} else {
		printf("\tOther mode: %llu\n", mode);
	}

	// enable tagging w/ 4-bit tags (MT_MODE=2)
	printf("Enabling MT_MODE\n");
	uint64_t val;
	
	asm volatile ("csrr %0, 0x30A" : "=r"(val) );  // menvcfg
	printf("\tread menvcfg=%p\n", val);
	val |= (2ull << 34);
	asm volatile ("csrw 0x30A, %0" :: "r"(val) );  // menvcfg
	printf("\twriting menvcfg=%p\n", val);
	
	asm volatile ("csrr %0, 0x30A" : "=r"(val) );  // menvcfg 
	printf("\tread menvcfg=%p\n", val);
	
	// check & set MVITT
	printf("Setting the VITT base register\n");
	asm volatile ("csrr %0, 0x34C" : "=r"(val) );  // mvitt
	printf("\tread mvitt=%p\n", val);

	uint64_t VITT_BASE = 0x80100000;
	asm volatile ("csrw 0x34c, %0" :: "r"(VITT_BASE) );  // menvcfg
	printf("\twriting mvitt=%p\n", VITT_BASE);
	
	asm volatile ("csrr %0, 0x34C" : "=r"(VITT_BASE) );  // mvitt
	printf("\tread mvitt=%p\n", VITT_BASE);

	// SET TAG
	uint8_t stored_tag;
	uint64_t vitt_entry;

	printf("Testing SETTAG\n");

	gentag_ptr = (long *) ((uintptr_t) genttag_ptr | (uintptr_t) ptr);
	printf("\tgentag_ptr set to %p\n", genttag_ptr);

	vitt_entry = VITT_BASE + (((uintptr_t)gentag_ptr & ~(0xFULL << 60)) >> 5);

	asm volatile ("lb %0, 0(%1)" : "=r" (stored_tag) : "r" (vitt_entry));
	printf("\treading from vitt address %p: %x\n", vitt_entry, stored_tag);

	printf("\texecuting \"settag gentag_ptr, 0\"\n");
	asm volatile ("settag %0, 0" :: "r" (gentag_ptr));
	printf("\texpecting tag=%x @ vaddr=%p\n", (uintptr_t)gentag_ptr>>60, vitt_entry);

	asm volatile ("lb %0, 0(%1)" : "=r" (stored_tag) : "r" (vitt_entry));
	printf("\treading from vitt address %p: %x\n", vitt_entry, stored_tag);

	return 0;
}
