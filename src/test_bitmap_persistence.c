#include <stdio.h>
#include <stdint.h>

#include "fs.h"
#include "disk.h"
#include "bitmap.h"

int main(void)
{
    uint8_t bitmap[FS_BLOCK_SIZE];

    /* Open fs.img again */
    int fd = disk_open("fs.img");

    if (fd == -1)
    {
        return 1;
    }

    /* Read the bitmap from disk */
    if (disk_read_block(fd, 5, bitmap) == -1)
    {
        disk_close(fd);
        return 1;
    }

    /* Check block 6 */
    printf("After reopening fs.img:\n");
    printf("Block 6 allocated? %d\n",
           bitmap_is_allocated(bitmap, 6));

    disk_close(fd);

    return 0;
}
