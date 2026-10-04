#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "fs.h"
#include "bitmap.h"

int main(void)
{
    uint8_t bitmap[FS_BLOCK_SIZE];

    /* Start with all blocks free */
    memset(bitmap, 0, sizeof(bitmap));

    /* Allocate two blocks */
    uint32_t block1 = bitmap_alloc(bitmap);
    uint32_t block2 = bitmap_alloc(bitmap);

    printf("Allocated block 1: %u\n", block1);
    printf("Allocated block 2: %u\n", block2);

    /* Check their allocation status */
    printf("Block %u allocated? %d\n",
           block1, bitmap_is_allocated(bitmap, block1));

    printf("Block %u allocated? %d\n",
           block2, bitmap_is_allocated(bitmap, block2));

    /* Free the first block */
    bitmap_free(bitmap, block1);

    printf("After freeing block %u:\n", block1);

    printf("Block %u allocated? %d\n",
           block1, bitmap_is_allocated(bitmap, block1));
    /* Allocate again after freeing block 6 */
    uint32_t block3 = bitmap_alloc(bitmap);

    printf("Allocated block 3: %u\n", block3);

    return 0;
}
