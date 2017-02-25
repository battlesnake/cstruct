#include "dynamic_array.h"

// struct buffer {
//   size_t capacity;  - number of items that can fit into current buffer
//   size_t length;    - number of items in current buffer
//   size_t alloc_by;  - number of items to grow buffer (capacity) by when growing automatically
//   size_t item_size; - number of bytes per item
//   void *data;       - pointer to buffer
//   void (*destructor)(void *) - item destructor
// };

// NOTE: When we destroy items, we call the destructor for them iff the destructor is not NULL.

typedef void (*destr)(void *);

// Initialiser - sets fields in instance and allocates buffer (via buffer_alloc)
void buffer_init(struct buffer *inst, size_t item_size, size_t capacity, 
                 size_t alloc_by, void (*destructor)(void *))
{   
    printf("%s\n", "buffer_init");
    inst->capacity   = 0;
    inst->length     = 0;
    inst->item_size  = item_size;
    inst->alloc_by   = alloc_by;
    inst->data       = NULL;
    inst->destructor = destructor;

    buffer_alloc(inst, capacity);
    printf("%s\n", "buffer_init");

}

// Destructor - frees the buffer, calling the destructor for each item in the buffer
void buffer_destroy(struct buffer *inst)
{
    printf("%s\n", "buffer_destroy");
    // if (inst->destructor) {
    //     ((destr)(inst->destructor))(inst->data);
    // }
    while(inst->data){
        if (inst->destructor) {
            //printf("Length before: %d\n", (int)inst->length);
            //printf("inst data: %p\n", inst->data);
            //printf("buff front: %p\n", buffer_front(inst));
            //((destr)(inst->destructor))(buffer_back(inst));
            if (inst->data == 0){
                free(inst->data);
            }
            inst->data += inst->item_size;
            //printf("%d\n", sizeof(inst->data));
            //assert(inst->item_size == sizeof(int*));
            --inst->length;
            //printf("bkbkb%d\n", (int)inst->length);
        } else{
            break;
        }
    }
    inst = NULL;
    printf("%s\n", "buffer_destroy");
}

// If realloc SUCCEDS we modify the size
static void try_realloc(struct buffer *inst, size_t new_capacity)
{
    printf("%s\n", "try_realloc");
    // void *succeds = realloc(inst->data, inst->item_size * new_capacity);
    // if(succeds){
    //     inst->data     = succeds;
    //     inst->capacity = new_capacity;
    //     if (inst->length > new_capacity) {
    //         inst->length = inst->capacity;
    //     }
    // } else {
    //     perror("Error - realloc failed!\n");
    // }

    for (size_t i = inst->capacity; i < new_capacity; ++i) {
        inst->data += i * inst->item_size;
        inst->data =  malloc(inst->item_size);
        if (!inst->data){
            break;
        }  
    }
    inst->capacity = new_capacity;
    if (inst->length > new_capacity) {
        inst->length = inst->capacity;
    }

    printf("%s\n", "try_realloc");
}

// Allocator
//  * Reallocates the buffer to a new_capacity, preserving existing items 
//     (unless new buffer is too small)
//  * Reduces length if new capacity is smaller than previous length
//  * For new capacity of zero, free() the buffer and set the pointer to NULL
//  * When reducing length, call destructor for any items which are being removed
void buffer_alloc(struct buffer *inst, size_t new_capacity)
{  
    printf("%s\n", "buffer_alloc");
    if (new_capacity == 0){
        buffer_destroy(inst);
    } else{
        // realloc will free the necessary memory
        try_realloc(inst, new_capacity);
    }
    printf("%s\n", "buffer_alloc");
}

// Grow the buffer if necessary, to ensure the requested minimum capacity
void buffer_reserve(struct buffer *inst, size_t min_capacity)
{
    printf("%s\n", "buffer_reserve");
    if(inst->capacity < min_capacity) {
        printf("%s\n", "buffer_reserve_iffff");
        try_realloc(inst, min_capacity);
    }
    printf("%s\n", "buffer_reserve");
}

// Change length of buffer
//  * If growing, grow capacity if required and zero-fill new items
//  * If shrinking, call destructor for any items which are being removed
void buffer_resize(struct buffer *inst, size_t length)
{   
    printf("%s\n", "buffer_resize");
    length +=  inst->length;
    size_t buffer_length = buffer_size(inst);
    // growing
    if (buffer_length < length) {
        printf("%s\n", "buffer_resize_iffff");
        buffer_reserve(inst, length);
        // for(int i = buffer_length; i < length; ++i){
        //     *(int*)(inst->data + i * inst->item_size) = 0;
        // }
    } else if (buffer_length > length) {
        buffer_alloc(inst, length);
    }
    printf("%s\n", "buffer_resize");
}

// Append to buffer (if no space, grow buffer by alloc_by items)
void buffer_push_back(struct buffer *inst, void *item) 
{   
    printf("%s\n", "buffer_push_back");
    size_t length = buffer_size(inst);
    printf("Length : %d\n", (int)length);
    ++inst->length;
    if (length <= inst->capacity) {
        void *new_item = buffer_at(inst, inst->length - 1);
        if (item) {
            memcpy(new_item, item, inst->item_size);
        } else {
            --inst->length;
        }
    } else {
        buffer_resize(inst, inst->alloc_by);
    }
    printf("Length after push : %d\n", (int)inst->length);
    printf("%s\n", "buffer_push_back");
}

// Pointer to first item in buffer
void *buffer_front(struct buffer *inst)
{
    if (inst) return inst->data;
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
    printf("%s\n", "buffer_pop");
    if (buffer_empty(inst)){
        return false;
    } else if (buffer_back(inst) == NULL) {
        return false;
    } else {
        memcpy(out, buffer_back(inst), inst->item_size);
        // TODO: call destructor on item
        // inst->destructor
        if (inst->destructor) {
            ((destr)(inst->destructor))(buffer_back(inst));
            --inst->length;
            //buffer_back(inst) = NULL;
        }
        printf("%s\n", "buffer_pop");
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

// Pointer to n'th item in buffer
void *buffer_at(struct buffer *inst, size_t index)
{
    if (index >= inst->length) {
        return NULL;
    }
    //printf("Buffer at: %d\n", (int)index);
    //printf("BUffert at(value)%d\n", *(int*)(inst->data + index * inst->item_size));
    return (void *)(inst->data + index * inst->item_size);
}

void destruct_size_t_item(void *item){
    free(item);
    //item = NULL;
}



// TESTING
//#if defined TEST_DYNAMIC_ARRAY


int main(){
    struct buffer inst;

    destr destruct_func = &destruct_size_t_item;
    buffer_init(&inst, sizeof(int*), 5, 2, destruct_func);
    assert(buffer_empty(&inst));
    assert(buffer_size(&inst) == 0);
    assert(item_size(&inst) == 8);
    assert(buffer_at(&inst, 89) == NULL);

    int *p = malloc(sizeof (int*));
    *p = 64;
    buffer_push_back(&inst, p);
    assert(!buffer_empty(&inst));
    assert(buffer_size(&inst) == 1);
    assert(*(int*)buffer_at(&inst, 0) == 64);
    assert(buffer_front(&inst) == buffer_back(&inst));
    printf("%d\n", *(int*)buffer_at(&inst, 0));

    // void *out = malloc(sizeof (int*));;
    // assert(buffer_pop(&inst, out));

    // assert(buffer_empty(&inst));
    // assert(buffer_size(&inst) == 0);
    // // printf("%d\n", *(int*)out);
    // assert(*(int*)out == 64);
    // free(out);



    int *t = malloc(sizeof (int*));
    *t = 128;
    buffer_push_back(&inst, t);
    // assert(buffer_size(&inst) == 1);
    // assert(*(int*)buffer_at(&inst, 0) == 128);
    // assert(buffer_front(&inst) == buffer_back(&inst));
    printf("%d\n", *(int*)buffer_at(&inst, 0));

    int *u = malloc(sizeof (int*));
    *u = 256;
    buffer_push_back(&inst, u);
    // assert(buffer_size(&inst) == 2);
    //assert(*(int*)buffer_at(&inst, 1) == 128);
    printf("%d\n", *(int*)buffer_at(&inst, 0));
    printf("%d\n", *(int*)buffer_at(&inst, 1));
    // assert(*(int*)buffer_at(&inst, 1) == 256);
    // assert(buffer_front(&inst) != buffer_back(&inst));

    int *m = malloc(sizeof (int*));
    *m = 512;
    buffer_push_back(&inst, m);
    //assert(buffer_size(&inst) == 3);
    //assert(*(int*)buffer_at(&inst, 1) == 128);
    printf("%d\n", *(int*)buffer_at(&inst, 0));
    printf("%d\n", *(int*)buffer_at(&inst, 1));
    printf("%d\n", *(int*)buffer_at(&inst, 2));
    printf("%d\n", *(int*)buffer_at(&inst, 3));
    // assert(*(int*)buffer_at(&inst, 1) == 256);
    // assert(buffer_front(&inst) != buffer_back(&inst));

    

    
    // free(p);
    // free(t);
    // free(u);
    // free(m);
    // buffer_destroy(&inst);
    
    return 0;
}

//#endif


//   A linked-list wrapper would have a similar interface, but there would be 
// no index/length/capacity fields/functions since these operations are not 
// trivial on a linked-list (and linked-lists don't have "capacity" since 
// they allocate one item at a time).