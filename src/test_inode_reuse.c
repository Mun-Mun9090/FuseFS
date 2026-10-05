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

    /* Free inode 1 */
    if (inode_free(fd, 1) == -1)
    {
        disk_close(fd);
        return 1;
    }

    printf("Inode 1 freed successfully.\n");

    /* Allocate another inode */
    uint32_t inode_number = inode_alloc(fd);

    if (inode_number == 0)
    {
        printf("No free inode available.\n");
        disk_close(fd);
        return 1;
    }

    printf("Allocated inode after freeing inode 1: %u\n",
           inode_number);

    disk_close(fd);

    return 0;
}
