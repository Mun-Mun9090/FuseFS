#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "fs.h"
#include "disk.h"
#include "inode.h"

int main(void)
{
    Inode inode;

    /* Open the filesystem image */
    int fd = disk_open("fs.img");

    if (fd == -1)
    {
        return 1;
    }

    /* Create an inode in memory */
    memset(&inode, 0, sizeof(Inode));

    inode.mode = 123;
    inode.uid = 1000;
    inode.gid = 1000;
    inode.link_count = 1;
    inode.size = 42;

    printf("Writing inode 1...\n");

    /* Write inode 1 to disk */
    if (inode_write(fd, 1, &inode) == -1)
    {
        disk_close(fd);
        return 1;
    }

    /* Clear our memory copy */
    memset(&inode, 0, sizeof(Inode));

    /* Read inode 1 back from disk */
    printf("Reading inode 1...\n");

    if (inode_read(fd, 1, &inode) == -1)
    {
        disk_close(fd);
        return 1;
    }

    /* Display the values read from disk */
    printf("Mode: %u\n", inode.mode);
    printf("UID: %u\n", inode.uid);
    printf("GID: %u\n", inode.gid);
    printf("Link count: %u\n", inode.link_count);
    printf("Size: %lu\n", inode.size);

    disk_close(fd);

    return 0;
}
