#ifndef INODE_H
#define INODE_H

#include <stdint.h>
#include "fs.h"

/*
 * Read an inode from the filesystem image.
 *
 * inode_number = inode number (0 to FS_INODE_COUNT - 1)
 *
 * Returns:
 *   0  on success
 *  -1  on failure
 */
int inode_read(int fd, uint32_t inode_number, Inode *inode);

/*
 * Write an inode to the filesystem image.
 *
 * inode_number = inode number (0 to FS_INODE_COUNT - 1)
 *
 * Returns:
 *   0  on success
 *  -1  on failure
 */
int inode_write(int fd, uint32_t inode_number, const Inode *inode);

/*
 * Find and allocate a free inode.
 *
 * Returns:
 *   inode number on success
 *   0 if no free inode is available
 */
uint32_t inode_alloc(int fd);

/*
 * Free an inode.
 */
int inode_free(int fd, uint32_t inode_number);

#endif
