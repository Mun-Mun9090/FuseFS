#ifndef DISK_H
#define DISK_H

#include <stdint.h>
#include <sys/types.h>

/*
 * Open the filesystem image.
 *
 * Returns:
 *   File descriptor on success.
 *   -1 on failure.
 */
int disk_open(const char *path);

/*
 * Close the filesystem image.
 */
void disk_close(int fd);

/*
 * Read one complete block from the filesystem image.
 *
 * block = physical block number.
 *
 * Returns:
 *   0 on success.
 *   -1 on failure.
 */
int disk_read_block(int fd, uint32_t block, void *buffer);

/*
 * Write one complete block to the filesystem image.
 *
 * block = physical block number.
 *
 * Returns:
 *   0 on success.
 *   -1 on failure.
 */
int disk_write_block(int fd, uint32_t block, const void *buffer);

#endif
