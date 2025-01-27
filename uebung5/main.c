#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"

int main(void) {
    FILE* file = fopen("output.txt", "w");

    fprintf(file, "===== Buddy Allocator Test =====\n\n");

    // 1. Initialize allocator
    fprintf(file, "[INIT]\n");
    mem_init();
    mem_dump(file);
    fprintf(file, "\n");

    // 2. Simple allocations
    fprintf(file, "[ALLOC 1]\n");
    void* p1 = mem_alloc(127);  // should need 2 pages
    void* p2 = mem_alloc(256);  // 5 pages required -> actual 8 pages (order 3)
    void* p3 = mem_alloc(72);   // 2 pages
    assert(p1 != NULL);
    assert(p2 != NULL);
    assert(p3 != NULL);
    mem_dump(file);
    fprintf(file, "\n");

    // 3. Free the middle block to test partial freeing
    fprintf(file, "[FREE p2]\n");
    mem_free(p2);
    mem_dump(file);
    fprintf(file, "\n");

    // 4. Allocate something larger
    fprintf(file, "[ALLOC 2]\n");
    void* p4 =
        mem_alloc(512);  // 512 + 1 byte header -> 9 pages required -> actual 16 pages (order 4)
    assert(p4 != NULL);
    mem_dump(file);
    fprintf(file, "\n");

    // 5. Free the first and third block
    fprintf(file, "[FREE p1 and p3]\n");
    mem_free(p1);
    mem_free(p3);
    mem_dump(file);
    fprintf(file, "\n");

    // 6. Allocate something big, just under a single block of order
    //    e.g. 800 bytes ->
    //    PAGE_SIZE=64 => 800+1 => 13 pages -> next power-of-two is 16 => order 4
    fprintf(file, "[ALLOC 3]\n");
    void* p5 = mem_alloc(800);
    assert(p5 != NULL);
    mem_dump(file);
    fprintf(file, "\n");

    // 7. Realloc that block to a bigger size
    //    e.g. 1600 bytes -> 25 pages -> next power-of-two is 32 => order 5
    fprintf(file, "[REALLOC p5 -> bigger]\n");
    memset(p5, 0xAB, 800);  // fill old space with some pattern
    void* p6 = mem_realloc(p5, 1600);
    assert(p6 != NULL);
    // check that data was copied
    unsigned char* checkBytes = (unsigned char*)p6;
    for (int i = 0; i < 800; i++) {
        assert(checkBytes[i] == 0xAB);
    }
    mem_dump(file);
    fprintf(file, "\n");

    // 8. Free the new block
    fprintf(file, "[FREE p6]\n");
    mem_free(p6);
    mem_dump(file);
    fprintf(file, "\n");

    fprintf(file, "All tests completed successfully.\n");
    return 0;
}
