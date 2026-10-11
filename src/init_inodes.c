#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#include "fs.h"

int main(void)
{
    Inode inodes[FS_INODE_COUNT];

    /* Start with all inodes empty */
    memset(inodes, 0, sizeof(inodes));

    /* Initialize inode 0 as the root directory */
    inodes[0].mode = S_IFDIR | 0755;
    inodes[0].uid = 0;
    inodes[0].gid = 0;
    inodes[0].link_count = 2;
    inodes[0].size = 0;

    time_t now = time(NULL);

    inodes[0].atime = (uint64_t)now;
    inodes[0].mtime = (uint64_t)now;
    inodes[0].ctime = (uint64_t)now;

    int fd = open("fs.img", O_RDWR);
    if (fd == -1)
    {
        perror("open");
        return 1;
    }

    ssize_t written = pwrite(fd,
                             inodes,
                             sizeof(inodes),
                             FS_BLOCK_SIZE);

    if (written != sizeof(inodes))
    {
        if (written == -1)
            perror("pwrite");
        else
            fprintf(stderr, "Incomplete inode table write\n");

        close(fd);
        return 1;
    }

    if (fsync(fd) == -1)
    {
        perror("fsync");
        close(fd);
        return 1;
    }

    close(fd);

    printf("Inode table initialized successfully.\n");

    return 0;
}
