
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
typedef struct {
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t link_count;
    uint64_t size;
    uint64_t atime;
    uint64_t mtime;
    uint64_t ctime;
    uint32_t direct_blocks[10];
    uint32_t indirect_block;
uint8_t reserved[32];
} Inode;

_Static_assert(sizeof(Inode) == 128,
               "Inode must be exactly 128 bytes");

#endif
