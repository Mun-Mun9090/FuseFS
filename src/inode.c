#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "fs.h"
#include "disk.h"
#include "inode.h"
#include "bitmap.h"


/*
 * Calculate which block contains a particular inode.
 */
static uint32_t inode_block(uint32_t inode_number)
{
    return (inode_number /
            (FS_BLOCK_SIZE / sizeof(Inode))) + 1;
}


/*
 * Calculate the byte offset of an inode inside its block.
 */
static uint32_t inode_offset(uint32_t inode_number)
{
    return (inode_number %
            (FS_BLOCK_SIZE / sizeof(Inode)))
           * sizeof(Inode);
}


/*
 * Read an inode from the filesystem image.
 */
int inode_read(int fd, uint32_t inode_number, Inode *inode)
{
    if (inode_number >= FS_INODE_COUNT || inode == NULL)
    {
        fprintf(stderr, "inode_read: invalid inode number\n");
        return -1;
    }

    uint8_t block[FS_BLOCK_SIZE];

    uint32_t block_number =
        inode_block(inode_number);

    if (disk_read_block(fd, block_number, block) == -1)
    {
        return -1;
    }

    uint32_t offset =
        inode_offset(inode_number);

    memcpy(inode,
           block + offset,
           sizeof(Inode));

    return 0;
}


/*
 * Write an inode to the filesystem image.
 */
int inode_write(int fd,
                uint32_t inode_number,
                const Inode *inode)
{
    if (inode_number >= FS_INODE_COUNT || inode == NULL)
    {
        fprintf(stderr, "inode_write: invalid inode number\n");
        return -1;
    }

    uint8_t block[FS_BLOCK_SIZE];

    uint32_t block_number =
        inode_block(inode_number);

    /*
     * Read the complete inode-table block first.
     * This is important because the block contains
     * multiple inodes.
     */
    if (disk_read_block(fd, block_number, block) == -1)
    {
        return -1;
    }

    uint32_t offset =
        inode_offset(inode_number);

    /*
     * Replace only the requested inode.
     */
    memcpy(block + offset,
           inode,
           sizeof(Inode));

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
 * Allocate a free inode with the specified
 * file type and permissions.
 */
uint32_t inode_alloc_mode(int fd, uint32_t mode)
{
    Inode inode;

    /*
     * Inode 0 is reserved for the root inode.
     */
    for (uint32_t i = 1;
         i < FS_INODE_COUNT;
         i++)
    {
        if (inode_read(fd, i, &inode) == -1)
        {
            return 0;
        }

        /*
         * mode == 0 means this inode is unused.
         */
        if (inode.mode == 0)
        {
            /*
             * Clear the entire inode first.
             */
            memset(&inode, 0, sizeof(Inode));

            /*
             * Store the file type and permissions.
             */
            inode.mode = mode;

            /*
             * Regular files initially have one link.
             * Directories initially have two links:
             *
             * .  -> itself
             * .. -> parent
             */
            if ((mode & S_IFMT) == S_IFDIR)
            {
                inode.link_count = 2;
            }
            else
            {
                inode.link_count = 1;
            }

            /*
             * Initialize timestamps.
             */
            time_t now = time(NULL);

            inode.atime = (uint64_t)now;
            inode.mtime = (uint64_t)now;
            inode.ctime = (uint64_t)now;

            /*
             * Write the newly allocated inode.
             */
            if (inode_write(fd, i, &inode) == -1)
            {
                return 0;
            }

            return i;
        }
    }

    /*
     * No free inode exists.
     */
    return 0;
}


/*
 * Allocate a free inode.
 *
 * Default:
 *   Regular file
 *   Permissions 0644
 */
uint32_t inode_alloc(int fd)
{
    return inode_alloc_mode(fd,
                            S_IFREG | 0644);
}


/*
 * Free all data blocks belonging to an inode.
 */
static int inode_free_blocks(int fd,
                             const Inode *inode)
{
    uint8_t bitmap[FS_BLOCK_SIZE];

    /*
     * Block 5 contains the bitmap.
     */
    if (disk_read_block(fd,
                        FS_DATA_START - 1,
                        bitmap) == -1)
    {
        return -1;
    }

    /*
     * Free all direct blocks.
     */
    for (int i = 0; i < 10; i++)
    {
        if (inode->direct_blocks[i] != 0)
        {
            bitmap_free(bitmap,
                        inode->direct_blocks[i]);
        }
    }

    /*
     * Free the indirect block and
     * all data blocks referenced by it.
     */
    if (inode->indirect_block != 0)
    {
        uint32_t entries[
            FS_BLOCK_SIZE / sizeof(uint32_t)
        ];

        /*
         * Read the indirect block.
         */
        if (disk_read_block(fd,
                            inode->indirect_block,
                            entries) == -1)
        {
            return -1;
        }

        uint32_t entry_count =
            FS_BLOCK_SIZE / sizeof(uint32_t);

        /*
         * Free every indirect data block.
         */
        for (uint32_t i = 0;
             i < entry_count;
             i++)
        {
            if (entries[i] != 0)
            {
                bitmap_free(bitmap,
                            entries[i]);
            }
        }

        /*
         * Finally free the indirect block itself.
         */
        bitmap_free(bitmap,
                    inode->indirect_block);
    }

    /*
     * Save the updated bitmap.
     */
    if (disk_write_block(fd,
                         FS_DATA_START - 1,
                         bitmap) == -1)
    {
        return -1;
    }

    return 0;
}


/*
 * Free an inode and all blocks belonging to it.
 */
int inode_free(int fd,
               uint32_t inode_number)
{
    /*
     * Inode 0 is reserved for root.
     */
    if (inode_number == 0 ||
        inode_number >= FS_INODE_COUNT)
    {
        fprintf(stderr,
                "inode_free: invalid inode number\n");

        return -1;
    }

    Inode inode;

    /*
     * Read the inode first.
     */
    if (inode_read(fd,
                   inode_number,
                   &inode) == -1)
    {
        return -1;
    }

    /*
     * Free all data blocks.
     */
    if (inode_free_blocks(fd,
                          &inode) == -1)
    {
        return -1;
    }

    /*
     * Clear the inode.
     */
    memset(&inode,
           0,
           sizeof(Inode));

    /*
     * Write the empty inode back.
     */
    if (inode_write(fd,
                    inode_number,
                    &inode) == -1)
    {
        return -1;
    }

    return 0;
}


/*
 * Allocate one direct data block for an inode.
 */
int inode_alloc_block(int fd,
                      uint32_t inode_number)
{
    Inode inode;

    /*
     * Read the inode.
     */
    if (inode_read(fd,
                   inode_number,
                   &inode) == -1)
    {
        return -1;
    }

    /*
     * Find an empty direct block pointer.
     */
    int free_index = -1;

    for (int i = 0; i < 10; i++)
    {
        if (inode.direct_blocks[i] == 0)
        {
            free_index = i;
            break;
        }
    }

    /*
     * All direct block pointers are full.
     */
    if (free_index == -1)
    {
        fprintf(stderr,
                "inode_alloc_block: "
                "no direct block available\n");

        return -1;
    }

    /*
     * Read the bitmap.
     */
    uint8_t bitmap[FS_BLOCK_SIZE];

    if (disk_read_block(fd,
                        FS_DATA_START - 1,
                        bitmap) == -1)
    {
        return -1;
    }

    /*
     * Allocate a free data block.
     */
    uint32_t block =
        bitmap_alloc(bitmap);

    if (block == 0)
    {
        fprintf(stderr,
                "inode_alloc_block: "
                "no free data block\n");

        return -1;
    }

    /*
     * Store the allocated block number
     * in the direct pointer.
     */
    inode.direct_blocks[free_index] =
        block;

    /*
     * Save the updated inode.
     */
    if (inode_write(fd,
                    inode_number,
                    &inode) == -1)
    {
        /*
         * The bitmap has only been changed
         * in memory, so there is no persistent
         * bitmap change to roll back.
         */
        return -1;
    }

    /*
     * Save the updated bitmap.
     */
    if (disk_write_block(fd,
                         FS_DATA_START - 1,
                         bitmap) == -1)
    {
        /*
         * The inode was written successfully,
         * but the bitmap write failed.
         *
         * Roll back the inode pointer.
         */
        inode.direct_blocks[free_index] = 0;

        if (inode_write(fd,
                        inode_number,
                        &inode) == -1)
        {
            fprintf(stderr,
                    "inode_alloc_block: "
                    "rollback failed\n");
        }

        return -1;
    }

    return 0;
}


/*
 * Allocate an indirect block for an inode.
 */
int inode_alloc_indirect_block(int fd,
                               uint32_t inode_number)
{
    Inode inode;

    /*
     * Read the inode.
     */
    if (inode_read(fd,
                   inode_number,
                   &inode) == -1)
    {
        return -1;
    }

    /*
     * Only one indirect block is currently supported.
     */
    if (inode.indirect_block != 0)
    {
        fprintf(stderr,
                "inode_alloc_indirect_block: "
                "indirect block already exists\n");

        return -1;
    }

    /*
     * Read the bitmap.
     */
    uint8_t bitmap[FS_BLOCK_SIZE];

    if (disk_read_block(fd,
                        FS_DATA_START - 1,
                        bitmap) == -1)
    {
        return -1;
    }

    /*
     * Allocate a block for the indirect block.
     */
    uint32_t block =
        bitmap_alloc(bitmap);

    if (block == 0)
    {
        fprintf(stderr,
                "inode_alloc_indirect_block: "
                "no free block\n");

        return -1;
    }

    /*
     * Start with an empty indirect block.
     */
    uint8_t indirect[FS_BLOCK_SIZE] = {0};

    /*
     * Write the empty indirect block.
     */
    if (disk_write_block(fd,
                         block,
                         indirect) == -1)
    {
        /*
         * The bitmap was changed only in memory,
         * so there is nothing persistent to roll back.
         */
        return -1;
    }

    /*
     * Store the indirect block number
     * inside the inode.
     */
    inode.indirect_block = block;

    /*
     * Write the updated inode.
     */
    if (inode_write(fd,
                    inode_number,
                    &inode) == -1)
    {
        return -1;
    }

    /*
     * Save the updated bitmap.
     */
    if (disk_write_block(fd,
                         FS_DATA_START - 1,
                         bitmap) == -1)
    {
        /*
         * Roll back the inode pointer.
         */
        inode.indirect_block = 0;

        if (inode_write(fd,
                        inode_number,
                        &inode) == -1)
        {
            fprintf(stderr,
                    "inode_alloc_indirect_block: "
                    "rollback failed\n");
        }

        return -1;
    }

    return 0;
}


/*
 * Allocate one data block through the inode's
 * indirect block.
 */
int inode_alloc_indirect_data_block(
    int fd,
    uint32_t inode_number)
{
    Inode inode;

    /*
     * Read the inode.
     */
    if (inode_read(fd,
                   inode_number,
                   &inode) == -1)
    {
        return -1;
    }

    /*
     * An indirect block must already exist.
     */
    if (inode.indirect_block == 0)
    {
        fprintf(stderr,
                "inode_alloc_indirect_data_block: "
                "no indirect block exists\n");

        return -1;
    }

    /*
     * Read the indirect block.
     */
    uint32_t entries[
        FS_BLOCK_SIZE / sizeof(uint32_t)
    ];

    if (disk_read_block(fd,
                        inode.indirect_block,
                        entries) == -1)
    {
        return -1;
    }

    uint32_t entry_count =
        FS_BLOCK_SIZE / sizeof(uint32_t);

    /*
     * Find an empty entry.
     */
    int free_index = -1;

    for (uint32_t i = 0;
         i < entry_count;
         i++)
    {
        if (entries[i] == 0)
        {
            free_index = (int)i;
            break;
        }
    }

    /*
     * Indirect block is full.
     */
    if (free_index == -1)
    {
        fprintf(stderr,
                "inode_alloc_indirect_data_block: "
                "indirect block is full\n");

        return -1;
    }

    /*
     * Read the bitmap.
     */
    uint8_t bitmap[FS_BLOCK_SIZE];

    if (disk_read_block(fd,
                        FS_DATA_START - 1,
                        bitmap) == -1)
    {
        return -1;
    }

    /*
     * Allocate a free data block.
     */
    uint32_t block =
        bitmap_alloc(bitmap);

    if (block == 0)
    {
        fprintf(stderr,
                "inode_alloc_indirect_data_block: "
                "no free data block\n");

        return -1;
    }

    /*
     * Put the allocated block number
     * into the indirect block.
     */
    entries[free_index] = block;

    /*
     * Save the updated indirect block.
     */
    if (disk_write_block(fd,
                         inode.indirect_block,
                         entries) == -1)
    {
        /*
         * Bitmap was changed only in memory.
         */
        return -1;
    }

    /*
     * Save the updated bitmap.
     */
    if (disk_write_block(fd,
                         FS_DATA_START - 1,
                         bitmap) == -1)
    {
        /*
         * Roll back the indirect block entry.
         */
        entries[free_index] = 0;

        if (disk_write_block(fd,
                             inode.indirect_block,
                             entries) == -1)
        {
            fprintf(stderr,
                    "inode_alloc_indirect_data_block: "
                    "rollback failed\n");
        }

        return -1;
    }

    return 0;
}
