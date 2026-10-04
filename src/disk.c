#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "fs.h"
#include "disk.h"

/*
 * Open the filesystem image.
 *
 * O_RDWR means:
 *   - we can read from fs.img
 *   - we can write to fs.img
 */
int disk_open(const char *path)
{
    int fd = open(path, O_RDWR);

    if (fd == -1)
    {
        perror("disk_open: open");
        return -1;
    }

    return fd;
}

/*
 * Close the filesystem image.
 */
void disk_close(int fd)
{
    if (close(fd) == -1)
    {
        perror("disk_close: close");
    }
}

/*
 * Read one complete filesystem block.
 */
int disk_read_block(int fd, uint32_t block, void *buffer)
{
    if (block >= FS_TOTAL_BLOCKS)
    {
        fprintf(stderr, "disk_read_block: invalid block %u\n", block);
        return -1;
    }

    off_t offset = (off_t)block * FS_BLOCK_SIZE;

    ssize_t bytes_read = pread(fd, buffer, FS_BLOCK_SIZE, offset);

    if (bytes_read == -1)
    {
        perror("disk_read_block: pread");
        return -1;
    }

    if (bytes_read != FS_BLOCK_SIZE)
    {
        fprintf(stderr,
                "disk_read_block: incomplete read (%zd bytes)\n",
                bytes_read);
        return -1;
    }

    return 0;
}

/*
 * Write one complete filesystem block.
 */
int disk_write_block(int fd, uint32_t block, const void *buffer)
{
    if (block >= FS_TOTAL_BLOCKS)
    {
        fprintf(stderr, "disk_write_block: invalid block %u\n", block);
        return -1;
    }

    off_t offset = (off_t)block * FS_BLOCK_SIZE;

    ssize_t bytes_written =
        pwrite(fd, buffer, FS_BLOCK_SIZE, offset);

    if (bytes_written == -1)
    {
        perror("disk_write_block: pwrite");
        return -1;
    }

    if (bytes_written != FS_BLOCK_SIZE)
    {
        fprintf(stderr,
                "disk_write_block: incomplete write (%zd bytes)\n",
                bytes_written);
        return -1;
    }

    if (fsync(fd) == -1)
    {
        perror("disk_write_block: fsync");
        return -1;
    }

    return 0;
}
