#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "myalloc.h"

static int tests_passed = 0;
static int tests_failed = 0;


/* Print the result of a test */
static void print_result(const char *name, int passed)
{
    if (passed)
    {
        printf("[PASS] %s\n", name);
        tests_passed++;
    }
    else
    {
        printf("[FAIL] %s\n", name);
        tests_failed++;
    }
}


/* Test 1: Basic allocation */
static void test_basic_allocation(void)
{
    void *ptr = myalloc(100);

    int passed = (ptr != NULL);

    print_result("Basic allocation", passed);

    if (ptr != NULL)
        mfree(ptr);
}


/* Test 2: Multiple allocations */
static void test_multiple_allocations(void)
{
    void *a = myalloc(64);
    void *b = myalloc(128);
    void *c = myalloc(256);

    int passed =
        (a != NULL &&
         b != NULL &&
         c != NULL);

    print_result("Multiple allocations", passed);

    if (a != NULL)
        mfree(a);

    if (b != NULL)
        mfree(b);

    if (c != NULL)
        mfree(c);
}


/* Test 3: Different allocation sizes */
static void test_different_sizes(void)
{
    void *small = myalloc(1);
    void *medium = myalloc(512);
    void *large = myalloc(4096);

    int passed =
        (small != NULL &&
         medium != NULL &&
         large != NULL);

    print_result("Different allocation sizes", passed);

    if (small != NULL)
        mfree(small);

    if (medium != NULL)
        mfree(medium);

    if (large != NULL)
        mfree(large);
}


/* Test 4: Write and read allocated memory */
static void test_write_read(void)
{
    char *ptr = (char *)myalloc(64);

    int passed = 0;

    if (ptr != NULL)
    {
        const char *message = "myalloc test";

        strcpy(ptr, message);

        if (strcmp(ptr, message) == 0)
            passed = 1;
    }

    print_result("Write and read allocated memory", passed);

    if (ptr != NULL)
        mfree(ptr);
}


/* Test 5: Memory pattern verification */
static void test_memory_patterns(void)
{
    unsigned char *ptr =
        (unsigned char *)myalloc(256);

    int passed = (ptr != NULL);

    if (ptr != NULL)
    {
        for (size_t i = 0; i < 256; i++)
            ptr[i] = (unsigned char)(i % 256);

        for (size_t i = 0; i < 256; i++)
        {
            if (ptr[i] != (unsigned char)(i % 256))
            {
                passed = 0;
                break;
            }
        }
    }

    print_result(
        "Memory write pattern verification",
        passed
    );

    if (ptr != NULL)
        mfree(ptr);
}


/* Test 6: Zero-size allocation */
static void test_zero_size(void)
{
    void *ptr = myalloc(0);

    /*
     * Our allocator intentionally returns NULL
     * for zero-byte allocations.
     */
    int passed = (ptr == NULL);

    print_result("Zero-size allocation", passed);

    if (ptr != NULL)
        mfree(ptr);
}


/* Test 7: Large allocation */
static void test_large_allocation(void)
{
    const size_t size = 1024 * 1024;

    unsigned char *ptr =
        (unsigned char *)myalloc(size);

    int passed = (ptr != NULL);

    if (ptr != NULL)
    {
        ptr[0] = 0xAA;
        ptr[size - 1] = 0x55;

        passed =
            (ptr[0] == 0xAA &&
             ptr[size - 1] == 0x55);
    }

    print_result("Large allocation (1 MB)", passed);

    if (ptr != NULL)
        mfree(ptr);
}


/* Test 8: Repeated allocations */
static void test_repeated_allocations(void)
{
    enum
    {
        BLOCK_COUNT = 100
    };

    void *blocks[BLOCK_COUNT];

    int passed = 1;
    int allocated = 0;

    for (int i = 0; i < BLOCK_COUNT; i++)
    {
        blocks[i] = myalloc(64);

        if (blocks[i] == NULL)
        {
            passed = 0;
            break;
        }

        allocated++;
    }

    for (int i = 0; i < allocated; i++)
        mfree(blocks[i]);

    print_result(
        "100 repeated allocations",
        passed
    );
}


/* Test 9: Allocation after free */
static void test_free_and_reuse(void)
{
    void *first = myalloc(128);

    int passed = (first != NULL);

    if (first != NULL)
    {
        mfree(first);

        void *second = myalloc(128);

        passed = (second != NULL);

        if (second != NULL)
            mfree(second);
    }

    print_result(
        "Allocation after free",
        passed
    );
}


/* Test 10: Fragmentation and reuse */
static void test_fragmentation(void)
{
    void *a = myalloc(128);
    void *b = myalloc(256);
    void *c = myalloc(128);

    int passed =
        (a != NULL &&
         b != NULL &&
         c != NULL);

    if (a != NULL &&
        b != NULL &&
        c != NULL)
    {
        /*
         * Free the middle block.
         */
        mfree(b);

        /*
         * Allocate a smaller block.
         * This tests reuse of freed memory.
         */
        void *d = myalloc(128);

        if (d == NULL)
            passed = 0;

        if (d != NULL)
            mfree(d);
    }

    if (a != NULL)
        mfree(a);

    if (c != NULL)
        mfree(c);

    print_result(
        "Fragmentation and reuse pattern",
        passed
    );
}


/* Test 11: mfree(NULL) safety */
static void test_free_null(void)
{
    /*
     * mfree(NULL) should safely do nothing.
     */
    mfree(NULL);

    print_result(
        "mfree(NULL) safety",
        1
    );
}


/* Test 12: Stress test */
static void test_stress(void)
{
    enum
    {
        BLOCK_COUNT = 500
    };

    void *blocks[BLOCK_COUNT];

    int passed = 1;
    int allocated = 0;

    for (int i = 0; i < BLOCK_COUNT; i++)
    {
        size_t size =
            (size_t)((i % 10) + 1) * 32;

        blocks[i] = myalloc(size);

        if (blocks[i] == NULL)
        {
            passed = 0;
            break;
        }

        allocated++;

        /*
         * Write to the allocated memory.
         */
        memset(
            blocks[i],
            0xAA,
            size
        );
    }

    /*
     * Free all successfully allocated blocks.
     */
    for (int i = 0; i < allocated; i++)
        mfree(blocks[i]);

    print_result(
        "Stress test (up to 500 allocations)",
        passed
    );
}


/* Main test runner */
int main(void)
{
    printf("\n");
    printf("============================================\n");
    printf("       MYALLOC VERIFICATION SUITE\n");
    printf("============================================\n\n");

    test_basic_allocation();
    test_multiple_allocations();
    test_different_sizes();
    test_write_read();
    test_memory_patterns();
    test_zero_size();
    test_large_allocation();
    test_repeated_allocations();
    test_free_and_reuse();
    test_fragmentation();
    test_free_null();
    test_stress();

    printf("\n");
    printf("============================================\n");
    printf("              TEST SUMMARY\n");
    printf("============================================\n");

    printf("Tests passed : %d\n", tests_passed);
    printf("Tests failed : %d\n", tests_failed);

    printf(
        "Total tests  : %d\n",
        tests_passed + tests_failed
    );

    printf("--------------------------------------------\n");

    if (tests_failed == 0)
    {
        printf("FINAL RESULT: ALL TESTS PASSED\n");
    }
    else
    {
        printf("FINAL RESULT: TESTS FAILED\n");
    }

    printf("============================================\n\n");

    return (tests_failed == 0) ? 0 : 1;
}