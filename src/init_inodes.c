#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "fs.h"

int main(void)
{
    Inode inodes[FS_INODE_COUNT];
    memset(inodes, 0, sizeof(inodes));

    int fd = open("fs.img", O_RDWR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    ssize_t written = pwrite(fd, inodes, sizeof(inodes),
                             FS_BLOCK_SIZE);

    if (written != sizeof(inodes)) {
        if (written == -1)
            perror("pwrite");
        else
            fprintf(stderr, "Incomplete inode table write\n");
        close(fd);
        return 1;
    }

    if (fsync(fd) == -1) {
        perror("fsync");
        close(fd);
        return 1;
    }

    close(fd);
    printf("Inode table initialized successfully.\n");

    return 0;
}
