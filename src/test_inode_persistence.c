#include <stdio.h>
#include <stdint.h>

#include "fs.h"
#include "disk.h"
#include "inode.h"

int main(void)
{
    /* Open the filesystem image */
    int fd = disk_open("fs.img");

    if (fd == -1)
    {
        return 1;
    }

    /* Read inode 1 */
    Inode inode;

    if (inode_read(fd, 1, &inode) == -1)
    {
        disk_close(fd);
        return 1;
    }

    printf("Before closing:\n");
    printf("Inode 1 mode: %u\n", inode.mode);

    /* Close the image */
    disk_close(fd);

    /* Reopen the image */
    fd = disk_open("fs.img");

    if (fd == -1)
    {
        return 1;
    }

    /* Read inode 1 again */
    if (inode_read(fd, 1, &inode) == -1)
    {
        disk_close(fd);
        return 1;
    }

    printf("After reopening:\n");
    printf("Inode 1 mode: %u\n", inode.mode);

    disk_close(fd);

    return 0;
}
