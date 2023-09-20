#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

typedef struct header
{
    unsigned int size;
    struct header *next;
    unsigned char marked;
} header_t;

// global variables used to keep track of the used and free memory blocks in the heap
header_t *free_list = NULL;
header_t *used_list = NULL;

void mark_used_blocks();
void scan_block(header_t *block);
void scan_segments();
void scan_stack();

void heapinit(unsigned int size)
{
    // create initial header for the entire heap
    header_t *heap = (header_t *)malloc(size);
    heap->size = size - sizeof(header_t); // subtract size of header from total size
    heap->next = NULL;
    heap->marked = 0;
    // set the free list to point to the initial header
    free_list = heap;
}

void *malloc1(unsigned int size)
{
    header_t *current, *previous;
    // round up size to multiple of 4 for alignment
    size = (size + 3) & ~3;
    // search free list for a block that fits
    previous = NULL;
    current = free_list;
    while (current != NULL)
    {
        if (current->size >= size)
        {
            // found a block that fits
            if (current->size - size <= sizeof(header_t))
            {
                // block is too small to split, use it as-is
                if (previous != NULL)
                {
                    // remove block from free list
                    previous->next = current->next;
                }
                else
                {
                    // block is at the beginning of the free list
                    free_list = current->next;
                }
                // add block to used list
                current->next = used_list;
                current->marked = 0;
                used_list = current;
                return (void *)(current + 1); // return pointer to data part of block
            }
            else
            {
                // split block and add new block to free list
                header_t *new_block = (header_t *)((char *)current + size + sizeof(header_t));
                new_block->size = current->size - size - sizeof(header_t);
                new_block->next = current->next;
                new_block->marked = 0;
                current->size = size;
                if (previous != NULL)
                {
                    // add new block to free list
                    previous->next = new_block;
                }
                else
                {
                    // new block is at the beginning of the free list
                    free_list = new_block;
                }
                // add current block to used list
                current->next = used_list;
                current->marked = 0;
                used_list = current;
                return (void *)(current + 1); // return pointer to data part of block
            }
        }
        previous = current;
        current = current->next;
    }
    // no block that fits found in free list
    return NULL;
}

void gc()
{
    // mark all used blocks that are still reachable
    mark_used_blocks();
    // iterate through the used list and add unmarked blocks to the free list
    header_t *current = used_list;
    header_t *previous = NULL;
    while (current != NULL)
    {
        if (!current->marked)
        {
            if (previous != NULL)
            {
                previous->next = current->next;
            }
            else
            {
                used_list = current->next;
            }
            current->next = free_list;
            current->marked = 0;
            free_list = current;
        }
        previous = current;
        current = current->next;
    }
}

void mark_used_blocks()
{
    // mark all used blocks that are still reachable
    // scan BSS and data segments for pointers to used blocks
    scan_segments();
    // scan used chunks for pointers to used blocks
    header_t *current = used_list;
    while (current != NULL)
    {
        scan_block(current);
        current = current->next;
    }

    // scan stack for pointers to used blocks
    scan_stack();
}

void scan_block(header_t *block)
{
    // mark block as reachable
    block->marked = 1;
    // scan block for pointers
    unsigned int *p = (unsigned int *)(block + 1);
    unsigned int *end = (unsigned int *)((char *)block + block->size + sizeof(header_t));
    while (p < end)
    {
        header_t *used_block = (header_t *)((char *)p - sizeof(header_t));
        if (used_block >= used_list && used_block < free_list)
        {
            used_block->marked = 1;
        }
        p++;
    }
}

void scan_segments()
{
    // get start and end addresses of BSS and data segments
    extern char _end;
    extern char __bss_start;
    char *start = &_end;
    char *end = &__bss_start;

    // scan for pointers
    while (start < end)
    {
        header_t *block = (header_t *)((char *)(unsigned int *)start - sizeof(header_t));
        if (block >= used_list && block < free_list)
        {
            block->marked = 1;
        }
        start += sizeof(unsigned int);
    }
}

void scan_stack()
{
    void *p = NULL;
    // get current stack pointer
    asm volatile("movq %%rsp, %0"
                 : "=r"(p));
    // scan for pointers
    while (p < (void *)&p)
    {
        header_t *block = (header_t *)((char *)(unsigned int *)p - sizeof(header_t));
        if (block >= used_list && block < free_list)
        {
            block->marked = 1;
        }
        p += sizeof(unsigned int);
    }
}

int main()
{
    // initialize heap
    heapinit(1024);
    // allocate memory
    int *p1 = (int *)malloc1(sizeof(int));
    *p1 = 10;
    int *p2 = (int *)malloc1(sizeof(int));
    *p2 = 20;
    // use allocated memory
    printf("p1: %d, p2: %d\n", *p1, *p2);
    // run GC
    gc();
    // allocate more memory
    int *p3 = (int *)malloc1(sizeof(int));
    *p3 = 30;
    // use allocated memory
    int *p4[10];
    printf("p3: %d\n", *p3);
    for (int i = 0; i < 10; i++)
    {
        int *p = (int *)malloc1(sizeof(int));
        *p = i * 3000;
        // printf(" p: %d\n", *p);
        //  the amount of allocated memory
        printf("used: %d, free: %d", used_list->size, free_list->size);
    }
    gc();
    return 0;
}
