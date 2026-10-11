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
 * Allocate a free inode with the specified file type and permissions.
 */
uint32_t inode_alloc_mode(int fd, uint32_t mode);
/*
 * Free an inode.
 */
int inode_free(int fd, uint32_t inode_number);
int inode_alloc_block(int fd, uint32_t inode_number);
/*
 * Allocate an indirect block for an inode.
 *
 * Returns:
 *   0  on success
 *  -1  on failure
 */
int inode_alloc_indirect_block(int fd, uint32_t inode_number);
/*
 * Allocate one data block through the inode's indirect block.
 *
 * Returns:
 *   0  on success
 *  -1  on failure
 */
int inode_alloc_indirect_data_block(int fd, uint32_t inode_number);
#endif
