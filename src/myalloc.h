#ifndef MYALLOC_H
#define MYALLOC_H

#include <stddef.h>

/*
 * Allocate 'size' bytes of memory.
 *
 * Returns:
 *     Pointer to allocated memory on success
 *     NULL on failure
 */
void *myalloc(size_t size);


/*
 * Free memory previously allocated by myalloc().
 *
 * mfree(NULL) is safe and does nothing.
 */
void mfree(void *ptr);

#endif