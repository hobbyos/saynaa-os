#include "kernel/mem/pmm.h"

#include "kernel/mem/paging.h"
#include "kernel/utils/debug.h"
#include "libc/math.h"
#include "libc/string.h"

#define NTHBIT(n) ((uint32_t) 1 << n)

static uint32_t bitmap[1024 * 1024 / 32];
static uint32_t mem_size;
static uint32_t used_blocks;
static uint32_t max_blocks;
static uintptr_t kernel_end;

void mmap_set(uint32_t bit);
void mmap_unset(uint32_t bit);
uint32_t mmap_test(uint32_t bit);
uint32_t mmap_find_free();
uint32_t mmap_find_free_frame(uint32_t num);

/**
 * @brief Initializes the physical memory manager (PMM).
 *
 * This function sets up the PMM by computing the end of the kernel and GRUB modules
 * in physical memory, preparing the block allocation bitmap, and parsing the memory
 * map to mark valid areas as available.
 *
 * @param boot Pointer to the multiboot2 structure provided by the bootloader.
 *
 * The function performs the following steps:
 * 1. Computes the end of the kernel and GRUB modules in physical memory.
 * 2. Ensures the kernel end is correctly set.
 * 3. Checks if the kernel is too large for its initial mapping.
 * 4. Prepares the block allocation bitmap, marking all blocks as taken by default.
 * 5. Parses the memory map to mark valid areas as available and calculates the total
 *    available and unavailable memory.
 * 6. Protects low memory, the kernel, and its modules.
 *
 * The function also prints memory statistics, including available memory, unavailable
 * memory, and memory taken by modules.
 */
void init_pmm(mb2_t* boot) {
    // Compute where the kernel & GRUB modules end in physical memory
    mb2_tag_t* tag = boot->tags;

    do {
        if (tag->type == MB2_TAG_MODULE) {
            mb2_tag_module_t* mod = (mb2_tag_module_t*) tag;
            kernel_end = max(mod->mod_end, kernel_end);
        }

        tag = (mb2_tag_t*) ((uintptr_t) tag + align_to(tag->size, 8));
    } while (tag->type != MB2_TAG_END);

    // In the author's case, modules come after the kernel,
    // but there's no guarantee it'll always be true
    if (kernel_end < (uintptr_t) &__kernel_end_phys__) {
        kernel_end = (uintptr_t) &__kernel_end_phys__;
    }

    if ((uint32_t) &__kernel_end_virt__ > KERNEL_END_MAP) {
        kprintf_error("The kernel is too large for its initial mapping");
        abort();
    }

    // Prepare our block allocation bitmap
    memset(bitmap, 0xFF, sizeof(bitmap)); // Blocks are taken by default

    // Parse the memory map to mark valid areas as available
    uint64_t available = 0;
    uint64_t unavailable = 0;

    mb2_tag_mmap_t* mmap = (mb2_tag_mmap_t*) mb2_find_tag(boot, MB2_TAG_MMAP);
    mb2_mmap_entry_t* ent = mmap->entries;

    while ((uintptr_t) ent < (uintptr_t) mmap + mmap->header.size) {
        if (ent->type == MB2_MMAP_AVAIL) {
            pmm_init_region((uintptr_t) ent->base_addr, ent->length);
            available += ent->length;
        } else {
            unavailable += ent->length;
        }

        ent = (mb2_mmap_entry_t*) ((uintptr_t) ent + mmap->entry_size);
    }

    mem_size = available;
    max_blocks = mem_size / PMM_BLOCK_SIZE;
    used_blocks = max_blocks;

    // Protect low memory, our glorious kernel and its modules
    pmm_deinit_region(0, kernel_end);
    pmm_deinit_region((uintptr_t) boot, boot->total_size);

    kprintf_info("memory stats: available: \x1B[32m%d MiB\x1B[0m", available >> 20);
    kprintf_info("unavailable: \x1B[32m%d KiB\x1B[0m", unavailable >> 10);
    kprintf_info("taken by modules: \x1B[32m%d MiB\x1B[0m",
        (kernel_end - (uintptr_t) &__kernel_end_phys__) >> 20);
}

/* Returns the number of bytes allocated by the PMM.
 */
uint32_t pmm_used_memory() {
    return used_blocks * PMM_BLOCK_SIZE;
}

/* Returns the number of free bytes the PMM started with.
 */
uint32_t pmm_total_memory() {
    return mem_size;
}

/**
 * @brief Mark an area of physical memory as available.
 *
 * This function initializes a region of physical memory, marking it as available
 * for use by the physical memory manager (PMM). It takes the starting address
 * and the size of the region to be marked as available.
 *
 * @param addr The starting address of the memory region.
 * @param size The size of the memory region in bytes.
 *
 * @note This function ensures that the nullptr (address 0) is never mapped.
 */
void pmm_init_region(uintptr_t addr, uint32_t size) {
    uint32_t base_block = addr / PMM_BLOCK_SIZE;
    /* A region might be smaller than a block, yet span two: boundaries */
    uint32_t num = divide_up(size + addr % PMM_BLOCK_SIZE, PMM_BLOCK_SIZE);

    while (num-- > 0) {
        mmap_unset(base_block++);
    }

    // Never map the nullptr
    mmap_set(0);
}

/**
 * @brief Marks an area of physical memory as used.
 *
 * This function marks a specified region of physical memory as used by
 * setting the corresponding blocks in the memory map.
 *
 * @param addr The starting address of the memory region to mark as used.
 * @param size The size of the memory region to mark as used, in bytes.
 */
void pmm_deinit_region(uintptr_t addr, uint32_t size) {
    uint32_t base_block = addr / PMM_BLOCK_SIZE;
    uint32_t num = divide_up(size + addr % PMM_BLOCK_SIZE, PMM_BLOCK_SIZE);

    while (num-- > 0) {
        mmap_set(base_block++);
    }
}

/**
 * @brief Allocates a page of physical memory.
 *
 * This function allocates a page of physical memory by finding a free memory block,
 * marking it as used, and returning its address. If there are no free memory blocks
 * available, it prints an error message and aborts the operation.
 *
 * @return The address of the allocated memory block, or 0 if no free block is found.
 */
uintptr_t pmm_alloc_page() {
    if (max_blocks - used_blocks <= 0) {
        kprintf_error("kernel is out of physical memory!");
        abort();
    }

    uint32_t block = mmap_find_free();

    if (!block) {
        return 0;
    }

    mmap_set(block);

    return (uintptr_t) (block * PMM_BLOCK_SIZE);
}

/**
 * @brief Allocates a 4 MiB area of physical memory, aligned to 4 MiB.
 *
 * This function finds an 8 MiB section of memory, within which it is guaranteed
 * to find an address that is 4 MiB-aligned. It then returns that address and
 * marks the 4 MiB area as taken.
 *
 * @return The address of the allocated 4 MiB-aligned memory area, or 0 if
 *         allocation fails.
 */
uintptr_t pmm_alloc_aligned_large_page() {     // TODO: generalize
    if (max_blocks - used_blocks < 2 * 1024) { // 4MiB
        return 0;
    }

    uint32_t free_block = mmap_find_free_frame(2 * 1024);

    if (!free_block) {
        return 0;
    }

    uint32_t aligned_block = ((free_block / 1024) + 1) * 1024;

    for (int i = 0; i < 1024; i++) {
        mmap_set(aligned_block + i);
    }

    return (uintptr_t) (aligned_block * PMM_BLOCK_SIZE);
}

/**
 * @brief Allocates a specified number of pages.
 *
 * This function allocates a specified number of pages by finding free frames
 * and marking them as used in the memory map.
 *
 * @param num The number of pages to allocate.
 * @return The starting address of the allocated pages, or 0 if allocation fails.
 */
uintptr_t pmm_alloc_pages(uint32_t num) {
    if (max_blocks - used_blocks < num) {
        return 0;
    }

    uint32_t first_block = mmap_find_free_frame(num);

    if (!first_block) {
        return 0;
    }

    for (uint32_t i = 0; i < num; i++) {
        mmap_set(first_block + i);
    }

    return (uintptr_t) (first_block * PMM_BLOCK_SIZE);
}

/**
 * @brief Frees a single page.
 *
 * This function frees a single page by unsetting the corresponding bit in the memory map.
 *
 * @param addr The address of the page to free.
 */
void pmm_free_page(uintptr_t addr) {
    uint32_t block = addr / PMM_BLOCK_SIZE;
    mmap_unset(block);
}

/**
 * @brief Frees a specified number of pages.
 *
 * This function frees a specified number of pages starting from a given address
 * by unsetting the corresponding bits in the memory map.
 *
 * @param addr The starting address of the pages to free.
 * @param num The number of pages to free.
 */
void pmm_free_pages(uintptr_t addr, uint32_t num) {
    uint32_t first_block = addr / PMM_BLOCK_SIZE;

    for (uint32_t i = 0; i < num; i++) {
        mmap_unset(first_block + i);
    }
}

/**
 * @brief Sets a bit in the memory map.
 *
 * This function sets a bit in the memory map to mark a block as used.
 *
 * @param bit The bit to set in the memory map.
 */
void mmap_set(uint32_t bit) {
    bitmap[bit / 32] |= NTHBIT(bit % 32);
    used_blocks++;
}

/**
 * @brief Unsets a bit in the memory map.
 *
 * This function unsets a bit in the memory map to mark a block as free.
 *
 * @param bit The bit to unset in the memory map.
 */
void mmap_unset(uint32_t bit) {
    bitmap[bit / 32] &= ~NTHBIT(bit % 32);
    used_blocks--;
}

/**
 * @brief Tests if a bit is set in the memory map.
 *
 * This function tests if a bit is set in the memory map, indicating whether
 * a block is used or free.
 *
 * @param bit The bit to test in the memory map.
 * @return Non-zero if the bit is set, zero if the bit is not set.
 */
uint32_t mmap_test(uint32_t bit) {
    return bitmap[bit / 32] & NTHBIT(bit % 32);
}

/* Returns the index of the first free bit in the bitmap
 */
uint32_t mmap_find_free() {
    for (uint32_t i = 0; i < max_blocks / 32; i++) {
        if (bitmap[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < 32; j++) {
                if (!(bitmap[i] & NTHBIT(j))) {
                    return i * 32 + j;
                }
            }
        }
    }

    return 0;
}

/* Returns the first block of frame_size bits
 */
uint32_t mmap_find_free_frame(uint32_t frame_size) {
    uint32_t first = 0;
    uint32_t count = 0;

    for (uint32_t i = 0; i < max_blocks / 32; i++) {
        if (bitmap[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < 32; j++) {
                if (!(bitmap[i] & NTHBIT(j))) {
                    if (!first) {
                        first = i * 32 + j;
                    }

                    count++;
                } else {
                    first = 0;
                    count = 0;
                }

                if (count == frame_size) {
                    return first;
                }
            }
        } else {
            first = 0;
            count = 0;
        }
    }

    return 0;
}

/* Returns the first address after the kernel in physical memory.
 */
uintptr_t pmm_get_kernel_end() {
    return (uintptr_t) kernel_end + max_blocks / 8;
}
