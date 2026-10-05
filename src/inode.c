#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "fs.h"
#include "disk.h"
#include "inode.h"

/*
 * Calculate which physical block contains an inode.
 */
static uint32_t inode_block(uint32_t inode_number)
{
    return FS_BLOCK_SIZE / sizeof(Inode) *
           (inode_number / (FS_BLOCK_SIZE / sizeof(Inode)))
           + 1;
}

/*
 * Calculate the position of an inode inside its block.
 */
static uint32_t inode_offset(uint32_t inode_number)
{
    return (inode_number % (FS_BLOCK_SIZE / sizeof(Inode)))
           * sizeof(Inode);
}

/*
 * Read an inode from fs.img.
 */
int inode_read(int fd, uint32_t inode_number, Inode *inode)
{
    if (inode_number >= FS_INODE_COUNT || inode == NULL)
    {
        fprintf(stderr, "inode_read: invalid inode number\n");
        return -1;
    }

    uint8_t block[FS_BLOCK_SIZE];

    uint32_t block_number = inode_block(inode_number);

    if (disk_read_block(fd, block_number, block) == -1)
    {
        return -1;
    }

    uint32_t offset = inode_offset(inode_number);

    memcpy(inode, block + offset, sizeof(Inode));

    return 0;
}

/*
 * Write an inode to fs.img.
 */
int inode_write(int fd, uint32_t inode_number, const Inode *inode)
{
    if (inode_number >= FS_INODE_COUNT || inode == NULL)
    {
        fprintf(stderr, "inode_write: invalid inode number\n");
        return -1;
    }

    uint8_t block[FS_BLOCK_SIZE];

    uint32_t block_number = inode_block(inode_number);

    /*
     * Read the whole block first because it contains
     * multiple inodes.
     */
    if (disk_read_block(fd, block_number, block) == -1)
    {
        return -1;
    }

    uint32_t offset = inode_offset(inode_number);

    memcpy(block + offset, inode, sizeof(Inode));

    /*
     * Write the complete block back.
     */
    if (disk_write_block(fd, block_number, block) == -1)
    {
        return -1;
    }

    return 0;
}

/*
 * Find and allocate a free inode.
 *
 * Inode 0 is reserved for the root directory,
 * so allocation starts from inode 1.
 */
uint32_t inode_alloc(int fd)
{
    Inode inode;

    for (uint32_t i = 1; i < FS_INODE_COUNT; i++)
    {
        if (inode_read(fd, i, &inode) == -1)
        {
            return 0;
        }

        /*
         * mode == 0 means this inode is currently unused.
         */
        if (inode.mode == 0)
        {
            memset(&inode, 0, sizeof(Inode));

            /*
             * Mark the inode as allocated.
             *
             * A regular file will use a normal non-zero mode
             * later. For now, 1 simply means "allocated".
             */
            inode.mode = 1;

            if (inode_write(fd, i, &inode) == -1)
            {
                return 0;
            }

            return i;
        }
    }

    return 0;
}

/*
 * Free an inode.
 */
int inode_free(int fd, uint32_t inode_number)
{
    if (inode_number == 0 || inode_number >= FS_INODE_COUNT)
    {
        fprintf(stderr, "inode_free: invalid inode number\n");
        return -1;
    }

    Inode inode;

    memset(&inode, 0, sizeof(Inode));

    return inode_write(fd, inode_number, &inode);
}
