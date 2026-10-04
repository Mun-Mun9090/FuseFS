#include <stdint.h>
#include "fs.h"
#include "bitmap.h"

/*
 * Convert a physical data-block number
 * into a bitmap bit number.
 *
 * Data block 6 corresponds to bitmap bit 0.
 * Data block 7 corresponds to bitmap bit 1.
 */
static uint32_t block_to_bit(uint32_t block)
{
    return block - FS_DATA_START;
}

/*
 * Set a bitmap bit to 1.
 *
 * 1 means the block is allocated/used.
 */
static void set_bit(uint8_t *bitmap, uint32_t bit)
{
    bitmap[bit / 8] |= (uint8_t)(1u << (bit % 8));
}

/*
 * Clear a bitmap bit to 0.
 *
 * 0 means the block is free.
 */
static void clear_bit(uint8_t *bitmap, uint32_t bit)
{
    bitmap[bit / 8] &= (uint8_t)~(1u << (bit % 8));
}

/*
 * Check whether a bitmap bit is 1.
 */
static int get_bit(const uint8_t *bitmap, uint32_t bit)
{
    return (bitmap[bit / 8] >> (bit % 8)) & 1u;
}

/*
 * Find the first free data block.
 *
 * Returns:
 *   Physical block number if successful.
 *   0 if no free block exists.
 */
uint32_t bitmap_alloc(uint8_t *bitmap)
{
    for (uint32_t block = FS_DATA_START;
         block < FS_TOTAL_BLOCKS;
         block++)
    {
        uint32_t bit = block_to_bit(block);

        if (!get_bit(bitmap, bit))
        {
            set_bit(bitmap, bit);
            return block;
        }
    }

    return 0;
}

/*
 * Mark a data block as free.
 */
void bitmap_free(uint8_t *bitmap, uint32_t block)
{
    if (block < FS_DATA_START || block >= FS_TOTAL_BLOCKS)
        return;

    uint32_t bit = block_to_bit(block);
    clear_bit(bitmap, bit);
}

/*
 * Check whether a data block is allocated.
 *
 * Returns:
 *   1 → allocated
 *   0 → free
 */
int bitmap_is_allocated(const uint8_t *bitmap, uint32_t block)
{
    if (block < FS_DATA_START || block >= FS_TOTAL_BLOCKS)
        return 0;

    uint32_t bit = block_to_bit(block);
    return get_bit(bitmap, bit);
}
