#include "dynamic_array.h"
#include "cstd/log.c"

// NOTE: When we destroy items, we call the destructor for them 
// iff the destructor is not NULL.

typedef void (*destr)(void *);

// Initialiser - sets fields in instance and allocates buffer (via buffer_alloc)
void buffer_init(struct buffer *inst, size_t item_size, size_t capacity, 
                 size_t alloc_by, void (*destructor)(void *))
{   
    memset(inst, 0, sizeof(*inst));
    inst->capacity   = 0;
    inst->length     = 0;
    inst->item_size  = item_size;
    inst->alloc_by   = alloc_by;
    inst->data       = NULL;
    inst->destructor = destructor;
    inst->buffer     = NULL;

    buffer_alloc(inst, capacity);
}

// Destructor - frees the buffer, calling the destructor for each item in the buffer
void buffer_destroy(struct buffer *inst)
{
    for(int i = 0; i < inst->capacity; i++){
        ((destr)(inst->destructor))(inst->buffer[i]);
    }
    inst->length = 0;
    free(inst->buffer);
    inst = NULL;
}

// If realloc SUCCEDS we modify the size
static void try_realloc(struct buffer *inst, size_t new_capacity)
{
    inst->buffer = realloc_zero(inst->buffer, 
                                buffer_size(inst) * sizeof(void*),
                                new_capacity * sizeof(void*));
    for (size_t i = inst->capacity; i < new_capacity; ++i) {
        void *succeds =  realloc(inst->buffer[i], inst->item_size);
        if (succeds){
            inst->buffer[i] = succeds;
        } else {
            log_error("%s\n", "Realloc failed!");
            abort();
        }
    }  
    
    inst->capacity = new_capacity;
    if (inst->length > new_capacity) {
        inst->length = inst->capacity;
    }
}

// Allocator
//  * Reallocates the buffer to a new_capacity, preserving existing items 
//     (unless new buffer is too small)
//  * Reduces length if new capacity is smaller than previous length
//  * For new capacity of zero, free() the buffer and set the pointer to NULL
//  * When reducing length, call destructor for any items which are being removed
void buffer_alloc(struct buffer *inst, size_t new_capacity)
{  
    if (new_capacity == 0){
        buffer_destroy(inst);
    } else{
        // realloc will free the necessary memory
        try_realloc(inst, new_capacity);
    }
}

// Grow the buffer if necessary, to ensure the requested minimum capacity
void buffer_reserve(struct buffer *inst, size_t min_capacity)
{
    if(inst->capacity < min_capacity) {
        size_t previous_capacity = inst->capacity;
        inst->buffer = realloc_zero(inst->buffer, 
                       previous_capacity * sizeof(void*), 
                       min_capacity * sizeof(void*));
        
        for (size_t i = inst->capacity; i < min_capacity; ++i) {
            void *succeds =  realloc(inst->buffer[i], inst->item_size);
            if (succeds){
                inst->buffer[i] = succeds;
            } else {
                log_error("%s\n", "Realloc failed!");
                abort();
            }
        }
        inst->capacity = min_capacity;
    }
}

// If realloc needs to increase size, safely increase and 
// memset the new data to zero;
void* realloc_zero(void* old_buffer, size_t old_size, size_t new_size) 
{
    void* new_buffer = realloc(old_buffer, new_size);
    if (new_size > old_size && new_buffer) {
        size_t diff = new_size - old_size;
        void* start = new_buffer + old_size;
        memset(start, 0, diff);
    } else {
        exit(0);
    }
    return new_buffer;
}

// Change length of buffer
//  * If growing, grow capacity if required and zero-fill new items
//  * If shrinking, call destructor for any items which are being removed
void buffer_resize(struct buffer *inst, size_t new_length)
{   
    // growing
    if (inst->capacity < new_length) {
        buffer_reserve(inst, new_length);
    // shrinking
    } else if (inst->capacity > new_length) {
        for (size_t i = new_length; i < inst->capacity; ++i) {
            ((destr)(inst->destructor))(inst->buffer[i]);
        }
        void *succeds = realloc(inst->buffer, new_length *sizeof(void*));
        if (succeds) {
            inst->buffer = succeds;
        } else {
            exit(0);
        }
    }
    inst->capacity = new_length;
    if (inst->length > new_length) {
        inst->length = inst->capacity;
    }
}

// Append to buffer (if no space, grow buffer by alloc_by items)
void buffer_push_back(struct buffer *inst, void *item) 
{   
    size_t length = buffer_size(inst);
    ++inst->length;
    if (length < inst->capacity) {
        void *new_item = buffer_at(inst, length);
        if (item) {
            memcpy(new_item, item, inst->item_size);
        } else {
            --inst->length;
        }
    } else {
        buffer_reserve(inst, inst->alloc_by + length);
    }
}

// Pointer to first item in buffer
void *buffer_front(struct buffer *inst)
{
    if (inst) return inst->buffer[0];
}

// Pointer to last item in buffer
void *buffer_back(struct buffer *inst)
{
    if (inst) return buffer_at(inst, buffer_size(inst) - 1);
}

// Take last item from buffer
//  * return false if buffer is empty
//  * copy item to "out" if not null
//  * call destructor for item being removed
//  * reduce length by 1
//  * return true
bool buffer_pop(struct buffer *inst, void *out)
{
    if (buffer_empty(inst)){
        return false;
    } else if (!buffer_back(inst)) {
        return false;
    } else {
        memcpy(out, buffer_back(inst), inst->item_size);
        --inst->length;
        return true;
    }
}

// Return true if buffer is empty (length == 0)
bool buffer_empty(struct buffer *inst){
    if (inst){
       return (buffer_size(inst) == 0); 
    } else {
        exit(0);
    }

}

// Number of items in the buffer (length)
size_t buffer_size(struct buffer *inst)
{
    if (inst) {
        return inst->length;
    } else {
        exit(0);
    }
}

// Item size of the elements in the buffer(item_size)
size_t item_size(struct buffer *inst)
{
    if (inst) {
        return inst->item_size;
    } else {
        exit(0);
    }
}

// Pointer to nth item in buffer
void *buffer_at(struct buffer *inst, size_t index)
{
    if (index >= inst->length) {
        return NULL;
    }
    return (inst->buffer[index]);
}

void destruct_size_t_item(void *item){
    free(item);
}



// TESTING
#if defined TEST_DYNAMIC_ARRAY

struct buffer init()
{
    struct buffer inst;
    destr destruct_func = &destruct_size_t_item;
    buffer_init(&inst, sizeof(int*), 5, 2, destruct_func);
    return inst;
}

void test_init()
{
    struct buffer inst = init();
    assert(buffer_empty(&inst));
    assert(buffer_size(&inst) == 0);
    assert(item_size(&inst) == 8);
    assert(buffer_at(&inst, 89) == NULL);

    buffer_destroy(&inst);
}

void test_add_elem()
{   
    struct buffer inst = init();
    int *p = malloc(sizeof (int*));
    *p = 64;
    buffer_push_back(&inst, p);

    assert(!buffer_empty(&inst));
    assert(buffer_size(&inst) == 1);
    assert(*(int*)buffer_at(&inst, 0) == 64);
    assert(buffer_front(&inst) == buffer_back(&inst));
    free(p);
    buffer_destroy(&inst);
}

void test_pop_elem()
{
    struct buffer inst = init();
    int *p = malloc(sizeof (int*));
    *p = 64;
    buffer_push_back(&inst, p);

    void *out = malloc(sizeof (int*));
    assert(buffer_pop(&inst, out));

    assert(buffer_empty(&inst));
    assert(buffer_size(&inst) == 0);
    assert(*(int*)out == 64);

    free(out);
    free(p);
    buffer_destroy(&inst);
}

void test_resize()
{
    struct buffer inst = init();
    int *p = malloc(sizeof (int*));
    *p = 64;
    buffer_push_back(&inst, p);
    *p = 128;
    buffer_push_back(&inst, p);
    assert(buffer_size(&inst) == 2);

    buffer_resize(&inst, 1);
    assert(buffer_size(&inst) == 1);
    assert(*(int*)buffer_at(&inst, 0) == 64);
    assert(buffer_front(&inst) == buffer_back(&inst));

    free(p);
    buffer_destroy(&inst);
}

void test_reserve()
{
    struct buffer inst = init();
    int *p = malloc(sizeof (int*));
    *p = 64;
    buffer_push_back(&inst, p);
    *p = 128;
    buffer_push_back(&inst, p);
    buffer_resize(&inst, 1);

    buffer_reserve(&inst, 5);
    assert(buffer_size(&inst) == 1);
    assert(buffer_front(&inst) == buffer_back(&inst));
    assert((&inst)->capacity = 5);
    assert(*(int*)buffer_at(&inst, 0) == 64);

    free(p);
    buffer_destroy(&inst);
}

int main(){
    printf("%s\n", "Starting tests...");
    test_init();
    test_add_elem();
    test_pop_elem();
    test_resize();
    test_reserve();

    printf("%s\n", "All tests passed!");
    
    return 0;
}

#endif


//   A linked-list wrapper would have a similar interface, but there would be 
// no index/length/capacity fields/functions since these operations are not 
// trivial on a linked-list (and linked-lists don't have "capacity" since 
// they allocate one item at a time).