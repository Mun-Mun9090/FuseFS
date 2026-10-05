#include <stdio.h>
#include <stdint.h>

#include "fs.h"
#include "disk.h"
#include "inode.h"

int main(void)
{
    int fd = disk_open("fs.img");

    if (fd == -1)
    {
        return 1;
    }

    /* Allocate the first free inode */
    uint32_t inode_number = inode_alloc(fd);

    if (inode_number == 0)
    {
        printf("No free inode available.\n");
        disk_close(fd);
        return 1;
    }

    printf("Allocated inode: %u\n", inode_number);

    /* Read the inode back to verify it is marked allocated */
    Inode inode;

    if (inode_read(fd, inode_number, &inode) == -1)
    {
        disk_close(fd);
        return 1;
    }

    printf("Inode %u mode: %u\n",
           inode_number, inode.mode);

    disk_close(fd);

    return 0;
}
