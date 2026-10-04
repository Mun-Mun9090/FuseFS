#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "fs.h"
#include "disk.h"
#include "bitmap.h"

int main(void)
{
    uint8_t bitmap[FS_BLOCK_SIZE];

    /* Open the filesystem image */
    int fd = disk_open("fs.img");

    if (fd == -1)
    {
        return 1;
    }

    /* Read the bitmap from block 5 */
    if (disk_read_block(fd, 5, bitmap) == -1)
    {
        disk_close(fd);
        return 1;
    }

    printf("Before allocation:\n");
    printf("Block 6 allocated? %d\n",
           bitmap_is_allocated(bitmap, 6));

    /* Allocate one data block */
    uint32_t block = bitmap_alloc(bitmap);

    printf("Allocated block: %u\n", block);

    printf("After allocation:\n");
    printf("Block %u allocated? %d\n",
           block, bitmap_is_allocated(bitmap, block));

    /* Write the modified bitmap back to block 5 */
    if (disk_write_block(fd, 5, bitmap) == -1)
    {
        disk_close(fd);
        return 1;
    }

    disk_close(fd);

    printf("Bitmap written back to fs.img successfully.\n");

    return 0;
}
