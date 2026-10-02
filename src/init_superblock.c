
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "fs.h"

int main(void)
{
    Superblock sb;
    memset(&sb, 0, sizeof(sb));

    sb.magic = FS_MAGIC;
    sb.version = FS_VERSION;
    sb.block_size = FS_BLOCK_SIZE;
    sb.total_blocks = FS_TOTAL_BLOCKS;
    sb.inode_count = FS_INODE_COUNT;
    sb.inode_table_start = 1;
    sb.bitmap_start = 5;
    sb.data_start = 6;

    int fd = open("fs.img", O_RDWR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    ssize_t written = pwrite(fd, &sb, sizeof(sb), 0);
    if (written != sizeof(sb)) {
        if (written == -1)
            perror("pwrite");
        else
            fprintf(stderr, "Incomplete superblock write\n");
        close(fd);
        return 1;
    }

    if (fsync(fd) == -1) {
        perror("fsync");
        close(fd);
        return 1;
    }

    close(fd);
    printf("Superblock written successfully.\n");

    return 0;
}
