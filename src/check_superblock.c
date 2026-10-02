#include <stdio.h>
#include "fs.h"

int main(void)
{
    printf("Superblock size: %zu bytes\n",
           sizeof(Superblock));
printf("Inode size: %zu bytes\n", sizeof(Inode));

    return 0;
}
