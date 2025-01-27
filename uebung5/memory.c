// memory.c

#include "memory.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>  // for memcpy

#include "bitset.h"

#define ORDER_MAX 10
#define PAGE_SIZE 64
#define HEAP_SIZE (PAGE_SIZE << ORDER_MAX)
#define HEADER_SIZE 1
#define NUM_BLOCKS (HEAP_SIZE / PAGE_SIZE)

static char heap[HEAP_SIZE];
static Bitset blocks[NUM_BLOCKS];

/**
 * Calculate the smallest order such that 2^order pages can accommodate the requested size.
 */
static char calculate_order(size_t size) {
    size_t total_size = size + HEADER_SIZE;
    int required_pages = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;

    char order = 0;
    while ((1 << order) < required_pages && order < ORDER_MAX) {
        order++;
    }

    return order;
}

/**
 * Check if a range of pages is free.
 */
static bool is_range_free(const Bitset *bits, int start, int length) {
    for (int i = 0; i < length; i++) {
        if (bitsetGet(bits, start + i)) {
            return false;
        }
    }
    return true;
}

/**
 * Determine the largest buddy-aligned free block starting at a given index.
 * Returns the order of the block or -1 if not free or misaligned.
 */
static char find_buddy_order(const Bitset *bits, int index) {
    if (index < 0 || index >= NUM_BLOCKS || bitsetGet(bits, index)) {
        return -1;
    }

    char order = 0;
    while (order <= ORDER_MAX) {
        int block_size = 1 << order;

        if (index % block_size != 0 || (index + block_size) > NUM_BLOCKS) {
            break;
        }

        if (!is_range_free(bits, index, block_size)) {
            break;
        }

        order++;
    }

    return (order > 0) ? (order - 1) : -1;
}

void mem_init(void) { bitsetInit(blocks, NUM_BLOCKS, 0); }

void *mem_alloc(size_t size) {
    char required_order = calculate_order(size);
    if (required_order > ORDER_MAX) {
        errno = ENOMEM;
        return NULL;
    }

    int required_pages = 1 << required_order;
    char best_order = ORDER_MAX + 1;
    int best_index = -1;

    for (int idx = 0; idx + required_pages <= NUM_BLOCKS;) {
        if (is_range_free(blocks, idx, required_pages)) {
            char current_order = find_buddy_order(blocks, idx);
            if (current_order >= required_order && current_order < best_order) {
                best_order = current_order;
                best_index = idx;
                if (best_order == required_order) {
                    break;
                }
            }

            if (current_order > -1) {
                idx += (1 << current_order);
            } else {
                idx++;
            }
        } else {
            idx++;
        }
    }

    if (best_index == -1) {
        errno = ENOMEM;
        return NULL;
    }

    for (int i = 0; i < required_pages; i++) {
        bitsetSet(blocks, best_index + i);
    }

    heap[best_index * PAGE_SIZE] = required_order;
    return &heap[best_index * PAGE_SIZE + HEADER_SIZE];
}

void *mem_realloc(void *ptr, size_t new_size) {
    if (!ptr) {
        return mem_alloc(new_size);
    }

    if (new_size == 0) {
        mem_free(ptr);
        return NULL;
    }

    char old_order = *((char *)ptr - HEADER_SIZE);
    size_t old_size = ((size_t)1 << old_order) * PAGE_SIZE - HEADER_SIZE;

    if (new_size <= old_size) {
        return ptr;
    }

    void *new_ptr = mem_alloc(new_size);
    if (!new_ptr) {
        errno = ENOMEM;
        return NULL;
    }

    memcpy(new_ptr, ptr, old_size);
    mem_free(ptr);
    return new_ptr;
}

void mem_free(void *ptr) {
    if (!ptr) {
        return;
    }

    char order = *((char *)ptr - HEADER_SIZE);
    int free_blocks = 1 << order;
    int block_idx = ((char *)ptr - heap - HEADER_SIZE) / PAGE_SIZE;

    if (block_idx < 0 || block_idx + free_blocks > NUM_BLOCKS) {
        return;
    }

    for (int i = 0; i < free_blocks; i++) {
        if (!bitsetGet(blocks, block_idx + i)) {
            return;
        }
    }

    for (int i = 0; i < free_blocks; i++) {
        bitsetClear(blocks, block_idx + i);
    }
}

void mem_dump(FILE *file) {
    fprintf(file, "Memory Dump:\n");
    for (int i = 0; i < NUM_BLOCKS; i++) {
        fprintf(file, "Block %d: %s\n", i, bitsetGet(blocks, i) ? "Allocated" : "Free");
    }
}
