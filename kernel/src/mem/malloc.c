#include "kernel/mem/malloc.h"

#include "kernel/mem/paging.h"
#include "kernel/utils/debug.h"
#include "libc/math.h"
#include "libc/string.h"

#define MIN_ALIGN 4

#define offsetof(t, d) __builtin_offsetof(t, d)

typedef struct _mem_block_t {
    struct _mem_block_t* next;
    uint32_t size; // We use the last bit as a 'used' flag
    uint8_t data[1];
} mem_block_t;

static mem_block_t* bottom = NULL;
static mem_block_t* top = NULL;
static uint32_t used_memory = 0;

/* Debugging function to print the block list. Only sizes are listed, and a '#'
 * indicates a used block.
 */
void mem_print_blocks() {
    mem_block_t* block = bottom;

    while (block) {
        kprintf("0x%x%s-> ", block->size & ~1, block->size & 1 ? "# " : " ");

        if (block->next && block->next < block) {
            kprintf("chaining error: block overlaps with previous one\n");
        }

        block = block->next;
    }

    kprintf("none\n");
}

/* Returns the size of a block, including the header.
 */
uint32_t mem_block_size(mem_block_t* block) {
    return sizeof(mem_block_t) + (block->size & ~1);
}

/* Returns the block corresponding to `pointer`, given that `pointer` was
 * previously returned by a call to `kmalloc`.
 */
mem_block_t* mem_get_block(void* pointer) {
    uintptr_t addr = (uintptr_t) pointer;

    return (mem_block_t*) (addr - sizeof(mem_block_t) + 4);
}

/* Appends a new block of the desired size and alignment to the block list.
 * Note: may insert an intermediary block before the one returned to prevent
 * memory fragmentation. Such a block would be aligned to `MIN_ALIGN`.
 */
mem_block_t* mem_new_block(uint32_t size, uint32_t align) {
    const uint32_t header_size = offsetof(mem_block_t, data);

    // I did the math and we always have next_aligned >= next.
    uintptr_t next = (uintptr_t) top + mem_block_size(top);
    uintptr_t next_aligned = align_to(next + header_size, align) - header_size;

    mem_block_t* block = (mem_block_t*) next_aligned;
    block->size = size | 1;
    block->next = NULL;

    // Insert a free block between top and our aligned block, if there's enough
    // space. That block is 8-bytes aligned.
    next = align_to(next + header_size, MIN_ALIGN) - header_size;
    if (next_aligned - next > sizeof(mem_block_t) + MIN_ALIGN) {
        mem_block_t* filler = (mem_block_t*) next;
        filler->size = next_aligned - next - sizeof(mem_block_t);
        top->next = filler;
        top = filler;
    }

    top->next = block;
    top = block;

    return block;
}

/* Returns whether the memory pointed to by `block` is a multiple of `align`.
 */
bool mem_is_aligned(mem_block_t* block, uint32_t align) {
    uintptr_t addr = (uintptr_t) block->data;

    return addr % align == 0;
}

/* Searches the block list for a free block of at least `size` and with the
 * proper alignment. Returns the first corresponding block if any, NULL
 * otherwise.
 */
mem_block_t* mem_find_block(uint32_t size, uint32_t align) {
    if (!bottom) {
        return NULL;
    }

    mem_block_t* block = bottom;

    while ((block->size & ~1) < size || block->size & 1 || !mem_is_aligned(block, align)) {
        block = block->next;

        if (!block) {
            return NULL;
        }
    }

    return block;
}

/* Returns a pointer to a memory area of at least `size` bytes.
 */
void* kmalloc(size_t size) {
    // Accessing basic datatypes at unaligned addresses is apparently undefined
    // behavior. Four-bytes alignement should be enough for most things.
    return aligned_alloc(MIN_ALIGN, size);
}

/**
 * @brief Reallocates memory block.
 *
 * This function changes the size of the memory block pointed to by `ptr` to `size` bytes.
 * The contents will be unchanged to the minimum of the old and new sizes; newly allocated memory will be uninitialized.
 *
 * @param ptr Pointer to the memory block to be reallocated. If NULL, the function behaves like kmalloc.
 * @param size New size of the memory block in bytes. If size is 0 and ptr is not NULL, the function behaves like free.
 * @return Pointer to the newly allocated memory block, or NULL if the allocation fails or if size is 0 and ptr is not NULL.
 */
void* krealloc(void* ptr, size_t size) {
    if (!ptr) {
        return kmalloc(size);
    }

    if (!size) {
        kfree(ptr);
        return NULL;
    }

    void* new = kmalloc(size);
    memcpy(new, ptr, mem_get_block(ptr)->size & ~1);
    kfree(ptr);

    return new;
}

/* Frees a pointer previously returned by `kmalloc`.
 */
void kfree(void* pointer) {
    if (!pointer) {
        return;
    }

    mem_block_t* block = mem_get_block(pointer);
    block->size &= ~1;
    used_memory -= block->size;
}

/**
 * @brief Allocates a block of memory with the specified alignment.
 *
 * This function allocates a block of memory of the given size, ensuring that the
 * starting address of the block is aligned to the specified alignment boundary.
 * If this is the first allocation, it sets up the initial block list.
 *
 * @param align The alignment boundary for the allocated memory block.
 * @param size The size of the memory block to allocate.
 * @return A pointer to the allocated memory block, or NULL if allocation fails.
 */
void* aligned_alloc(size_t align, size_t size) {
    const uint32_t header_size = offsetof(mem_block_t, data);
    size = align_to(size, 8);

    // If this is the first allocation, setup the block list:
    // it starts with an empty, used block, in order to avoid edge cases.
    if (!top) {
        uintptr_t addr = KERNEL_HEAP_BEGIN;
        uintptr_t heap_phys = pmm_alloc_pages(KERNEL_HEAP_SIZE / 0x1000);
        paging_map_pages(addr, heap_phys, KERNEL_HEAP_SIZE / 0x1000, PAGE_RW);

        bottom = (mem_block_t*) addr;
        top = bottom;
        top->size = 1; // That means used, of size 0
        top->next = NULL;
    }

    mem_block_t* block = mem_find_block(size, align);

    if (block) {
        used_memory += block->size;
        block->size |= 1;

        return block->data;
    } else {
        // We'll have to allocate a new block, so we check if we haven't
        // exceeded the memory we can distribute.
        uintptr_t end = (uintptr_t) top + mem_block_size(top) + header_size;
        end = align_to(end, align) + size;

        // The kernel can't allocate more
        if (end > KERNEL_HEAP_BEGIN + KERNEL_HEAP_SIZE) {
            kprintf_error("kernel ran out of memory!");
            abort();
        }
        block = mem_new_block(size, align);
    }

    used_memory += size;

    return block->data;
}

void* kamalloc(uint32_t size, uint32_t align) {
    return aligned_alloc(align, size);
}

/* Returns the memory allocated on the heap by the kernel, in bytes.
 */
uint32_t memory_usage() {
    return used_memory;
}