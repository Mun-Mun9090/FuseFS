
#ifndef FS_H
#define FS_H

#include <stdint.h>

#define FS_MAGIC        0x46555345
#define FS_VERSION      1
#define FS_BLOCK_SIZE   4096
#define FS_TOTAL_BLOCKS 4096
#define FS_INODE_COUNT  128

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t inode_count;
    uint32_t inode_table_start;
    uint32_t bitmap_start;
    uint32_t data_start;
    uint8_t reserved[FS_BLOCK_SIZE - 32];
} Superblock;

_Static_assert(sizeof(Superblock) == FS_BLOCK_SIZE,
               "Superblock must be exactly one block");

#endif
