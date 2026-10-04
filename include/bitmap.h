#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>

/*
 * Find a free data block in the bitmap.
 *
 * Returns:
 *   Physical block number if a free block is found.
 *   0 if no free block is available.
 */
uint32_t bitmap_alloc(uint8_t *bitmap);

/*
 * Mark a physical data block as free.
 */
void bitmap_free(uint8_t *bitmap, uint32_t block);

/*
 * Check whether a physical data block is currently allocated.
 *
 * Returns:
 *   1 if allocated
 *   0 if free
 */
int bitmap_is_allocated(const uint8_t *bitmap, uint32_t block);

#endif
