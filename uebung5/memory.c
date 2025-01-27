#include "memory.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

#include "bitset.h"

/**@brief Order of the largest possible memory block. */
#define ORDER_MAX 10

/**@brief Size of the smallest possible memory block. */
#define PAGE_SIZE 64

/**@brief Size of available memory. */
#define HEAP_SIZE (PAGE_SIZE << ORDER_MAX)

/**@brief Size of the header prepended to every block (invisible to user). */
#define HEADER_SIZE 1

#define NUM_BLOCKS (HEAP_SIZE / PAGE_SIZE)

/**@brief Heap memory. */
static char heap[HEAP_SIZE];

static Bitset blocks[NUM_BLOCKS];

static char calc_order(size_t size) {
    size_t needed_size = size + HEADER_SIZE;
    int req_pages = (needed_size + PAGE_SIZE - 1) / PAGE_SIZE;
    // = ceil(needed_size / PAGE_SIZE)

    char order = 0;
    int power_of_two = 1;
    while (power_of_two < req_pages) {
        power_of_two <<= 1;
        order++;
    }

    return order;
}

static bool is_free(Bitset *blocks, int block_idx, int block_size) {
    for (int i = block_idx; i < block_idx + block_size; i++) {
        if (bitsetGet(blocks, i)) {
            return false;
        }
    }
    return true;
}

/* @brief Finds the block order of a free block.
 * @param block_idx The first page index of a block.
 */
static char get_block_order(Bitset *blocks, int block_idx) {
    if (!is_free(blocks, block_idx, 1)) return -1;

    char order = 0;
    int step_size = 1;
    while (block_idx + step_size < NUM_BLOCKS && is_free(blocks, block_idx, step_size)) {
        // TODO: halve compares by removing multiple checks for the first pages of a block
        // while (block_idx + 2 * step_size < NUM_BLOCKS && is_free(blocks, block_idx + step_size,
        // step_size))

        // TODO: prune by checking if best_order is already better & by starting from req_blocks
        // (instead of 1)

        // FIXME: might give too large orders when finding consecutive free pages that aren't
        // necessarily in one block

        order++;
        step_size <<= 1;
    }

    order--;
    return order;
}

void mem_init() { bitsetInit(blocks, NUM_BLOCKS, 0); }

void *mem_alloc(size_t size) {
    char req_order = calc_order(size);
    if (req_order > ORDER_MAX) goto fail;
    int req_blocks = 1 << req_order;

    // search for smallest block that can accommodate
    char best_order = ORDER_MAX + 1;
    int best_block_idx = -1;

    int block_idx = 0;
    while (block_idx + req_blocks < NUM_BLOCKS) {
        if (is_free(blocks, block_idx, req_blocks)) {
            char block_order = get_block_order(blocks, block_idx);

            if (block_order < best_order) {
                best_order = block_order;
                best_block_idx = block_idx;
            }
            if (block_order == req_order) break;

            // TODO: make this more readable
            if (block_order > req_order) {
                block_order = req_order;
            }
            char rel_block_order = req_order - block_order;
            block_idx += (1 << rel_block_order) * req_blocks;
        } else {
            block_idx += req_blocks;
        }
    }

    if (best_block_idx == -1) goto fail;

    // block found! -> allocate memory
    for (int i = best_block_idx; i < best_block_idx + req_blocks; i++) {
        bitsetSet(blocks, i);
    }

    int header_idx = best_block_idx * PAGE_SIZE;
    heap[header_idx] = req_order;

    return &heap[header_idx + HEADER_SIZE];

fail:
    errno = ENOMEM;
    return NULL;
}

void *mem_realloc(void *oldptr, size_t new_size) {
    /* TODO: increase the size of an existing block through merging
     *   OR  allocate a new one with identical contents */
fail:
    errno = ENOMEM;
    return NULL;
}

void mem_free(void *ptr) {
    // TODO: check for validity of pointer

    char order = *((char *)ptr - HEADER_SIZE);
    int free_blocks = 1 << order;

    int block_idx = ((char *)ptr - &heap[0]) / 64;

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