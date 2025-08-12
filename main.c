#include "stdio.h"
#include "assert.h"
#include "stdbool.h"
#include <stdlib.h>

#define HEAP_CAP 640000 // stands for Heap capacity
#define CHUNK_LIST_CAP 1024

#define UNIMPLEMENTED() \
    do { \
        fprintf(stderr, "%s:%d: TODO: %s is not implemented yet\n", \
                __FILE__, __LINE__, __func__); \
        abort(); \
    } while(0)

typedef struct  {
    char *start;
    size_t size;
} Chunk;

typedef struct  {
    size_t count;
    Chunk chunks[CHUNK_LIST_CAP];
} Chunk_List;


void chunk_list_dump(const Chunk_List *list){
    printf("Chunks (%zu):\n", list->count);
    for (size_t i = 0; i < list->count; ++i){
        printf("start: %p, size: %zu\n",
                (void *) list->chunks[i].start,
                list->chunks[i].size);
    }
}

int chunk_start_compar(const void *a, const void *b)
{
    const Chunk *a_chunk = a;
    const Chunk *b_chunk = b;
    return a_chunk->start - b_chunk->start;
}

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
    assert(list->count < CHUNK_LIST_CAP);
    list->chunks[list->count].start = ptr;
    list->chunks[list->count].size = size;

    for (size_t i = list->count; 
            i > 0 && list->chunks[i].start < list->chunks[i-1].start; 
            --i){
                Chunk t = list->chunks[i];
                list->chunks[i] = list->chunks[i - 1];
                list->chunks[i - 1] = t;
    }
    list->count += 1;
}

void chunk_list_merge(Chunk_List *dst, Chunk_List *src)
{
    dst->count = 0;
    for (size_t i = 0; i < src->count; ++i){
        const Chunk chunk = src->chunks[i];
        if (dst->count > 0){
            Chunk *top_chunk = &dst->chunks[dst->count - 1];

            if (top_chunk->start + top_chunk->size ){
                top_chunk->size += chunk.size;
            }else {
                chunk_list_insert(dst, chunk.start, chunk.size);
            }
        } else {
            chunk_list_insert(dst, chunk.start, chunk.size);
        }
    }
}

void chunk_list_remove(Chunk_List *list, size_t index)
{
    assert(index < list->count);
    for (size_t i = index; i < list->count; ++i) {
        list->chunks[i] = list->chunks[i+1];
    }
    list->count -= 1;
}

char heap[HEAP_CAP] = {0};

Chunk_List alloced_chunks = {0};
Chunk_List freed_chunks = {
    .count = 1,
    .chunks = {
        [0] = {.start = heap, .size = sizeof(heap)},
    },
};
Chunk_List tmp_chunks = {0};

void *heap_alloc(size_t size)
{
    if (size > 0)
    {
        chunk_list_merge(&tmp_chunks, &freed_chunks);
        freed_chunks = tmp_chunks;
        for (size_t i = 0; i < freed_chunks.count; ++i)
        {
            const Chunk chunk = freed_chunks.chunks[i];
            if (chunk.size >= size)
            {
                chunk_list_remove(&freed_chunks, i);

                const size_t tail_size = chunk.size - size;

                chunk_list_insert(&alloced_chunks, chunk.start, size);

                if (tail_size > 0)
                {
                    chunk_list_insert(&freed_chunks, (char *)chunk.size + size, tail_size);
                }

                return chunk.start;
            }
        }
    }
    return NULL;
}

// O(Alloced)
void heap_free(void *ptr)
{
    if (ptr != NULL){
        const int index = chunk_list_find(&alloced_chunks, ptr);
        assert(index >= 0);
        assert(ptr == alloced_chunks.chunks[index].start);
        chunk_list_insert(&freed_chunks,
                          alloced_chunks.chunks[index].start,
                          alloced_chunks.chunks[index].size);
        chunk_list_remove(&alloced_chunks, (size_t)index);
    }
}

void heap_collect(void)
{

    UNIMPLEMENTED();
}

int main(void)
{
    void *ptrs[10] = {0};
    for (int i = 0; i < 10; ++i){
        ptrs[i] = heap_alloc(i);
    }
    for (int i = 0; i < 10; ++i){
        if (i % 2 == 0){
            heap_free(ptrs[i]);
        }
    }
    // for (size_t i = 1; i <= 10; ++i){
    //     void *p = heap_alloc(i);
    //     if (i % 2 == 0){
    //         heap_free(p);
    //     }
    // }

    heap_alloc(10);
    chunk_list_dump(&alloced_chunks);
    printf("Free ");
    chunk_list_dump(&freed_chunks);
    
    return 0;
}
