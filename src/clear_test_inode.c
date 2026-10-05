#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

    /* Clear inode 1 */
    if (inode_free(fd, 1) == -1)
    {
        disk_close(fd);
        return 1;
    }

    printf("Inode 1 cleared successfully.\n");

    disk_close(fd);

    return 0;
}
