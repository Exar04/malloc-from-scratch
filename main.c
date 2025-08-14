#include "stdio.h"
#include <malloc/malloc.h>
#include "stdlib.h"
#include "assert.h"


#define HEAP_CAPACITY 640000
#define CHUNK_LIST_CAP 1024

// This defines every chunk we allocate
// it stores the starting point of our chunk and the total size of the chunk
typedef struct  {
    char *start;
    size_t size;
} Chunk;

// chunk list stores the list of chunks
// we will be using it to store how many chunks we have allocated
// also how many chunks we have free
typedef struct  {
    size_t count;
    Chunk chunks[CHUNK_LIST_CAP];
} Chunk_List;

char heap[HEAP_CAPACITY] = {0};

// We are initilizing it with 0 as we have not allocated any chunks yet
Chunk_List allocated_chunks = {0};

// In freed_chunks we are initializing it with the whole heap as all the memory is free
// so we have one chunk which is the whole heap
// in output it will look like this:
// Chunks (1): start: 0x7f8b3c000000, size: 640000
Chunk_List freed_chunks = {
    .count = 1,
    .chunks = {
        [0] = {.start = heap, .size = sizeof(heap)},
    },
};

// the returns the index of the chunk based on input pointer
int chunk_list_find(const Chunk_List *list,  void *ptr)
{
    for (size_t i = 0; i < list->count; ++i){
        if (list->chunks[i].start == ptr) {
            return (int) i;
        }
    }
    return -1;
}

void chunk_list_insert(Chunk_List *list, void *ptr, size_t size)
{
    // we check if our list if full or not
    // if it is full we cannot insert any more chunks
    // so we will error out
    assert(list->count < CHUNK_LIST_CAP);

    // we insert the chunk at the end of the list
    // we are using the last index of the list to insert the chunk
    list->chunks[list->count].start = ptr;
    list->chunks[list->count].size = size;

    // we are doing mini bubble sort here
    // we are sorting the chunks based on their starting address
    // we always assume that our list is sorted
    // and now we have added new element at the end of the list
    // that element might be smaller from the previous elements
    // so we check from backwards and swap the elements until we find the right position
    
    // e.g. if we have chunks like this:
    // index:   0      1      2
    // start: 0x100  0x200  0x400

    // and now we add new element at index 3
    // index:   0      1      2      3
    // start: 0x100  0x200  0x400  0x250   (unsorted)
    //                               ^- this is the new element and added at the end of the list
    //                                  and teh address is smaller than the previous elements
    //                                  so we need to swap it

    // SWAP 1:
    // index:   0      1      2      3
    // start: 0x100  0x200  0x250  0x400   (unsorted)
    // now that we have swapped and the element, our inserted chunk is greater than the previous element and thus list is in sorted order
    for (size_t i = list->count; i > 0 && list->chunks[i].start < list->chunks[i - 1].start; --i)
    {
        Chunk t = list->chunks[i];
        list->chunks[i] = list->chunks[i - 1];
        list->chunks[i - 1] = t;
    }
    list->count += 1; // incrementing the count as we have added a new chunk
}

// merge function merges the 'adjecent' fragemented memory chunks
void chunk_list_merge(Chunk_List *dst, Chunk_List *src){
    // Given src:
        // index:   0            1            2
        // start: 0x1000      0x1100       0x2000
        // size:   0x100       0x100        0x080

    dst->count = 0;

    for (size_t i = 0; i < src->count; ++i)
    {
        const Chunk chunk = src->chunks[i];
        if (dst->count > 0)
        {
            Chunk *top_chunk = &dst->chunks[dst->count - 1];
            // we will get last chunk form dst list
            // and check if the start of the current chunk is adjacent to the last chunk
            // if it is adjacent, we will merge the two chunks
            // and add it to the last chunk
            // by adding the start and size of top chunk 
            if (top_chunk->start + top_chunk->size == chunk.start) // this part is kinda sketchy and should be checked
            {
                top_chunk->size += chunk.size;
            }
            else
            {
                // but if it is not adjacent, we will just insert the chunk into dst
                chunk_list_insert(dst, chunk.start, chunk.size);
            }
        }
        else
        {
            // since dst count is goint to be 0 at start
            // we will just insert the first chunk into dst
            chunk_list_insert(dst, chunk.start, chunk.size);
        }
    }
}

void chunk_list_remove(Chunk_List *list, size_t index)
{
    assert(index < list->count);
    for (size_t i = index; i < list->count-1; ++i) {
        list->chunks[i] = list->chunks[i+1];
    }
    list->count -= 1;
}

void *heap_alloc(size_t size)
{
    if (size > 0)
    {
        Chunk_List tmp_chunks = {0};
        chunk_list_merge(&tmp_chunks, &freed_chunks);
        freed_chunks = tmp_chunks;
        for (size_t i = 0; i < freed_chunks.count; ++i)
        {
            const Chunk chunk = freed_chunks.chunks[i];
            if (chunk.size >= size)
            {
                chunk_list_remove(&freed_chunks, i);

                const size_t tail_size = chunk.size - size;

                chunk_list_insert(&allocated_chunks, chunk.start, size);

                if (tail_size > 0)
                {
                    chunk_list_insert(&freed_chunks, (char *)chunk.start + size, tail_size);
                }

                return chunk.start;
            }
        }
    }
    return NULL;
}

// O(N)
void heap_free(void *ptr)
{
    if (ptr != NULL){
        const int index = chunk_list_find(&allocated_chunks, ptr);
        assert(index >= 0);
        assert(ptr == allocated_chunks.chunks[index].start);
        chunk_list_insert(&freed_chunks,
                          allocated_chunks.chunks[index].start,
                          allocated_chunks.chunks[index].size);
        chunk_list_remove(&allocated_chunks, (size_t)index);
    }
}

void chunk_list_dump(const Chunk_List *list){
    printf("Chunks (%zu):\n", list->count);
    for (size_t i = 0; i < list->count; ++i){
        printf("start: %p, size: %zu\n",
                (void *) list->chunks[i].start,
                list->chunks[i].size);
    }
}

int main(void){
    void *ptr1 = heap_alloc(10);
    void *ptr2 = heap_alloc(50);
    (void) ptr1;
    (void) ptr2;

    heap_free(ptr1);
    heap_alloc(10);
    heap_free(ptr2);
    // void *ptr3 = heap_alloc(10);
    // (void) ptr3;

    chunk_list_dump(&allocated_chunks);
    printf("Free");
    chunk_list_dump(&freed_chunks);

    return 0;
}
