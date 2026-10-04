#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "fs.h"
#include "disk.h"

int main(void)
{
    uint8_t buffer[FS_BLOCK_SIZE];

    /* Open the filesystem image */
    int fd = disk_open("fs.img");

    if (fd == -1)
    {
        return 1;
    }

    /* Read block 0 (the superblock) */
    if (disk_read_block(fd, 0, buffer) == -1)
    {
        disk_close(fd);
        return 1;
    }

    /* Interpret the block as a Superblock */
    Superblock *sb = (Superblock *)buffer;

    printf("Magic: 0x%08X\n", sb->magic);
    printf("Version: %u\n", sb->version);
    printf("Block size: %u\n", sb->block_size);
    printf("Total blocks: %u\n", sb->total_blocks);
    printf("Inode count: %u\n", sb->inode_count);
    printf("Bitmap start: %u\n", sb->bitmap_start);
    printf("Data start: %u\n", sb->data_start);

    /* Close the filesystem image */
    disk_close(fd);

    return 0;
}
