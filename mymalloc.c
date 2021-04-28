#include "mymalloc.h"
#include <unistd.h>

static int SIZE = 1024;

void* mymalloc(size_t size)
{
    static __uint8_t first_call = 1;/*ilk defa calisacaksa*/
    if(first_call == 1)
    {
        heap_start = sbrk(SIZE);
        heap_start->info.isfree = 1;
        heap_start->info.size = SIZE - sizeof(Block) - sizeof(Info);
        
        Info* tag = get_tag(heap_start);
        tag->size = heap_start->info.size;
        tag->isfree = 1;

        heap_start->next = NULL;
        heap_start->previous = NULL;
        free_list = heap_start;

        first_call = 0;
    }

    size = ((size + 15)/16)*16;


    if(heap_start == (void*)-1)
        return NULL;

    Block* find = NULL;
    if(strategy == firstFit)
    {
        find = first_fit(size);
    }

    else if(strategy == bestFit)
    {
        find = best_fit(size);
    }
    else
    {
        find = worst_fit(size);
    }
    
    /*bos alan bulunamadi*/
    if(find == NULL)
        return NULL;
    Block*p = split(find,size);
    
    return p->data;
}

void *myfree(void *p)
{
    if(p == NULL)
        return NULL;
    Block* ptr = (Block*)((char*)p - sizeof(Block));
    if(ptr->info.isfree == 1)
        return NULL;
    if(free_list == NULL)
    {
        free_list = ptr;
        ptr->next = NULL;
        ptr->previous = NULL;
        ptr->info.isfree = 1;
        get_tag(ptr)->isfree = 1;
    }
    
    else
    {
        Block* itr = free_list;
        while(itr->next != NULL)
        {
            itr = itr->next;
        }
        itr->next = ptr;
        ptr->previous = itr;
        ptr->next = NULL;
        ptr->info.isfree = 1;
        get_tag(ptr)->isfree = 1;
    }
    
    coalescing(ptr);
}

Block *split(Block *b, size_t size)
{
    int old_size = b->info.size+sizeof(Block)+sizeof(Info);
    int free_size = old_size - 2*sizeof(Block) - 2*sizeof(Info) - size;
    Block* s = b;
    if(free_size < 16)
        size = b->info.size;
    s->info.size = size;
    s->info.isfree = 0;
    Info* tag_s = get_tag(s);
    tag_s->isfree = 0;
    tag_s->size = size;

    
    Block* s2 = NULL;
    if(free_size > 0)
    {
        s2 = (Block*)((char*)tag_s + sizeof(Info));
        s2->info.size = free_size;
        s2->info.isfree = 1;
        Info* tag_s2 = get_tag(s2);
        tag_s2->isfree = 1;
        tag_s2->size = s2->info.size;
        s2->next = NULL;
        s2->previous = NULL;
    }
    delete_add_list(s,s2);
    
    return s;
}

void printHeap()
{
    Block*b = heap_start;
    printf("Blocks\n");
    printf("--------------------\n");
    while(b != NULL)
    {
        printf("Free: %d\nSize: %d\n" , b->info.isfree , b->info.size);
        printf("--------------------\n");
        b = right(b);
    }
}

Info* get_tag(Block* start)
{
    return (Info*)((char*)start + sizeof(Block) + start->info.size);
}

Block* first_fit(size_t size)
{
    Block* itr = free_list;

    while(itr != NULL)
    {
        if(itr->info.size >= size)
            return itr;
        itr = itr->next;
    }

    return NULL;
}

Block* best_fit(size_t size)
{
    Block* itr = free_list;
    Block* best = NULL;
    unsigned int best_size = __UINT32_MAX__; 
    while(itr != NULL)
    {
        if(itr->info.size >= size && itr->info.size < best_size)
        {
            best = itr;
            best_size = itr->info.size;
        }
        itr = itr->next;
    }

    return best;
}

Block* worst_fit(size_t size)
{
    Block* itr = free_list;
    Block* worst = NULL;
    unsigned int worst_size = 0; 
    while(itr != NULL)
    {
        if(itr->info.size >= size && itr->info.size > worst_size)
        {
            worst = itr;
            worst_size = itr->info.size;
        }
        itr = itr->next;
    }

    return worst;
}

void delete_add_list(Block* del , Block* add)
{
    if(del == free_list)
    {
        if(del->next == NULL)
        {
            free_list = add;
            return;
        }
        else
        {
            if(add == NULL)
            {
                free_list = del->next;
                free_list->previous = NULL;
                del->next = NULL;
                return;
            }
            else
            {
                free_list = add;
                add->next = del->next;
                add->next->previous = add;
                del->next = NULL;
                return;
            }
        }
    }

    else
    {
        if(add == NULL)
        {
            del->previous->next = del->next;
            if(del->next != NULL)
                del->next->previous = del->previous;
            del->next = NULL;
            del->previous = NULL;
            return;
        }
        else
        {
            del->previous->next = add;
            add->previous = del->previous;
            if(del->next != NULL)
                del->next->previous = add;
            add->next = del->next;
            del->next = NULL;
            del->previous = NULL;
        }
    }
}

void coalescing(Block* b)
{
    Block* l = left(b);
    Block* r = right(b);
    if(l == NULL && r == NULL)
        return;
    if(l == NULL || l!= NULL && l->info.isfree == 0)
        if(r == NULL || r!= NULL && r->info.isfree == 0)
            return;
    Block* start = NULL;
    if(l != NULL && l->info.isfree == 1)
        start = l;
    else
        start = b;
    if(l != NULL && l->info.isfree == 1)
        delete_list(l);
    delete_list(b);
    if(r != NULL && r->info.isfree == 1)
        delete_list(r);
    
    int size = 0;
    if(l != NULL && l->info.isfree == 1)
        size += (l->info.size + sizeof(Block) + sizeof(Info));
    size+= (b->info.size + sizeof(Block) + sizeof(Info));
    if(r != NULL && r->info.isfree == 1)
        size += (r->info.size + sizeof(Block) + sizeof(Info));

    start->info.size = size - sizeof(Info) - sizeof(Block);
    start->info.isfree = 1;
    start->next = NULL;
    start->previous = NULL;
    Info* tag = get_tag(start);
    tag->isfree = 1;
    tag->size = start->info.size;

    add_list(start);
}

void delete_list(Block* b)
{
    if(b == free_list)
    {
        free_list = free_list->next;
        if(free_list != NULL)
            free_list->previous = NULL;
        b->next = NULL;
    }

    else
    {
        if(b->next != NULL)
            b->next->previous = b->previous;
        b->previous->next = b->next;
        b->next = NULL;
        b->previous = NULL;
    } 
}

void add_list(Block* b)
{
    if(free_list == NULL)
        free_list = b;
    else
    {
        Block* itr = free_list;
        while(itr->next != NULL)
            itr = itr->next;
        itr->next = b;
        b->previous = itr;
    }
}

Block* left(Block* b)
{
    Info* tag = (Info*)((char*)b - sizeof(Info));
    if((char*)tag < (char*)heap_start)
        return NULL;
    return (Block*)((char*)tag - sizeof(Block) - tag->size);
}
Block* right(Block* b)
{
    Block* p = (Block*)((char*)get_tag(b) + sizeof(Info));
    if((char*)p >= (char*)heap_start + SIZE)
        return NULL;
    else
        return p;
}

void free_list_print()
{
  Block* itr = free_list;
    while(itr != NULL)
    {
        printf("%d\n" , get_tag(itr)->size);
        itr = itr->next;
    }
    printf("-------------------------\n");
}