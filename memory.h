#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"

struct E820 {
    uint64_t address;
    uint64_t length;
    uint32_t type;
} __attribute__((packed));

struct FreeMemRegion {
    uint64_t address;
    uint64_t length;
};

struct Page {
    struct Page* next;
};

typedef uint64_t PDE;
typedef PDE* PD;
typedef PD* PDPTR;

// P,W,U attributes in paging tables
#define PTE_P 1 // present
#define PTE_W 2 // writable
#define PTE_U 4 // accessible by user applications
#define PTE_ENTRY 0x80
#define KERNEL_BASE 0xffff800000000000
#define PAGE_SIZE (2*1024*1024)

// allign address to next 2mb boundry if not alligned
#define PA_UP(v) ((((uint64_t)v + PAGE_SIZE-1) >> 21) << 21)
// allign address to previous 2mb boundry
#define PA_DOWN(v) (((uint64_t)v >> 21) << 21)
// convert physical address to virtaul address
// by adding kernel base virtual address
#define P2V(p) ((uint64_t)(p) + KERNEL_BASE)
// convert virtual address to physical address
// by subtracting kernel base virtual address
#define V2P(v) ((uint64_t)(v) - KERNEL_BASE)
// retrives address of next level page
// clear lower 12 bits of PML4 or PDPT or PDT to do so
#define PDE_ADDR(p) (((uint64_t)p >> 12) << 12)
// retrives address of next level physical page
// clear lower 21 bits of Page table entry to do so
#define PTE_ADDR(p) (((uint64_t)p >> 21) << 21)

void* kalloc(void);
void kfree(uint64_t v);
void init_memory(void);
void init_kvm(void);
bool map_pages(uint64_t map, uint64_t v, uint64_t e, uint64_t pa, uint32_t attribute);
void switch_vm(uint64_t map);
void load_cr3(uint64_t map);
void free_vm(uint64_t map);
void free_page(uint64_t map, uint64_t v, uint64_t e);
bool setup_uvm(uint64_t map, uint64_t start, int size);
uint64_t setup_kvm(void);
uint64_t get_total_memory(void); 

#endif
