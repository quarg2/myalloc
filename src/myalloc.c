// 

#include "myalloc.h"

#include <stddef.h>
#include <unistd.h>

#define NALLOC 1024

/*
 * Header stored before every allocated/free block.
 *
 * size:
 *     Number of Header-sized units in the block.
 *
 * next:
 *     Pointer to the next block in the free list.
 *
 * max_align_t:
 *     Ensures that returned memory is properly aligned.
 */
typedef union Header
{
    struct
    {
        size_t size;
        union Header *next;
    } s;

    max_align_t align;

} Header;


/*
 * Base node for the circular free list.
 */
static Header base;


/*
 * Pointer to the current position in the free list.
 */
static Header *freep = NULL;


/*
 * Add a block to the free list.
 *
 * This function also coalesces adjacent free blocks.
 */
static void add_to_free_list(Header *bp)
{
    Header *p;

    /*
     * Find the correct position for bp
     * in the circular free list.
     */
    for (p = freep;
         !(bp > p && bp < p->s.next);
         p = p->s.next)
    {
        /*
         * Handle the case where the free list
         * wraps around the end of memory.
         */
        if (p >= p->s.next &&
            (bp > p || bp < p->s.next))
        {
            break;
        }
    }

    /*
     * Coalesce with the block immediately after bp.
     */
    if (bp + bp->s.size == p->s.next)
    {
        bp->s.size += p->s.next->s.size;
        bp->s.next = p->s.next->s.next;
    }
    else
    {
        bp->s.next = p->s.next;
    }

    /*
     * Coalesce with the block immediately before bp.
     */
    if (p + p->s.size == bp)
    {
        p->s.size += bp->s.size;
        p->s.next = bp->s.next;
    }
    else
    {
        p->s.next = bp;
    }

    /*
     * Update the free-list pointer.
     */
    freep = p;
}


/*
 * Request additional memory from the operating system.
 */
static Header *morecore(size_t nunits)
{
    char *cp;
    Header *up;

    /*
     * Request at least NALLOC units.
     */
    if (nunits < NALLOC)
        nunits = NALLOC;

    /*
     * Extend the program break.
     */
    cp = sbrk(nunits * sizeof(Header));

    if (cp == (char *)-1)
        return NULL;

    /*
     * Treat the newly allocated memory as
     * a free block.
     */
    up = (Header *)cp;
    up->s.size = nunits;

    /*
     * Add the new block to the free list.
     *
     * Do NOT call mfree() here because this block
     * was obtained directly from the operating system.
     */
    add_to_free_list(up);

    return freep;
}


/*
 * Allocate memory.
 */
void *myalloc(size_t size)
{
    Header *p;
    Header *prevp;

    /*
     * We choose to return NULL for zero-byte
     * allocations.
     */
    if (size == 0)
        return NULL;

    /*
     * Calculate the number of Header-sized units.
     *
     * +1 accounts for the Header itself.
     */
    size_t nunits =
        (size + sizeof(Header) - 1) /
        sizeof(Header) + 1;

    /*
     * Initialize the circular free list
     * on the first allocation.
     */
    if ((prevp = freep) == NULL)
    {
        base.s.next = &base;
        base.s.size = 0;

        freep = &base;
        prevp = &base;
    }

    /*
     * Search the free list.
     */
    for (p = prevp->s.next;
         ;
         prevp = p, p = p->s.next)
    {
        /*
         * Found a block large enough.
         */
        if (p->s.size >= nunits)
        {
            /*
             * Exact-size block.
             */
            if (p->s.size == nunits)
            {
                prevp->s.next = p->s.next;
            }
            else
            {
                /*
                 * Split the block.
                 *
                 * The remaining free portion stays
                 * in the free list.
                 */
                p->s.size -= nunits;

                p += p->s.size;

                p->s.size = nunits;
            }

            /*
             * Remember where the search stopped.
             */
            freep = prevp;

            /*
             * Return memory immediately after
             * the Header.
             */
            return (void *)(p + 1);
        }

        /*
         * Reached the end of the free list.
         * Request more memory.
         */
        if (p == freep)
        {
            p = morecore(nunits);

            if (p == NULL)
                return NULL;
        }
    }
}


/*
 * Free previously allocated memory.
 */
void mfree(void *ptr)
{
    Header *bp;

    /*
     * mfree(NULL) is safe.
     */
    if (ptr == NULL)
        return;

    /*
     * The Header is located immediately
     * before the memory returned to the user.
     */
    bp = (Header *)ptr - 1;

    /*
     * Return the block to the free list.
     *
     * add_to_free_list() also handles
     * adjacent-block coalescing.
     */
    add_to_free_list(bp);
}