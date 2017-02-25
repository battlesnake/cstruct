#pragma once
#include "cstd/std.h"

struct buffer {
  size_t capacity;  //- number of items that can fit into current buffer
  size_t length;    //- number of items in current buffer
  size_t alloc_by;  //- number of items to grow buffer (capacity) by when growing automatically
  size_t item_size; //- number of bytes per item
  void *data;       //- pointer to buffer
  void (*destructor)(void *);// - item destructor
};

// NOTE: When we destroy items, we call the destructor for them iff the destructor is not NULL.

// Initialiser - sets fields in instance and allocates buffer (via buffer_alloc)
void buffer_init(struct buffer *inst, size_t item_size, size_t capacity, size_t alloc_by, void (*destructor)(void *));

// Destructor - frees the buffer, calling the destructor for each item in the buffer
void buffer_destroy(struct buffer *inst);

// Allocator
//  * Reallocates the buffer to a new capacity, preserving existing items (unless new buffer is too small)
//  * Reduces length if new capacity is smaller than previous length
//  * For new capacity of zero, free() the buffer and set the pointer to NULL
//  * When reducing length, call destructor for any items which are being removed
void buffer_alloc(struct buffer *inst, size_t capacity);

// Grow the buffer if necessary, to ensure the requested minimum capacity
void buffer_reserve(struct buffer *inst, size_t min_capacity);

// Change length of buffer
//  * If growing, grow capacity if required and zero-fill new items
//  * If shrinking, call destructor for any items which are being removed
void buffer_resize(struct buffer *inst, size_t length);

// Append to buffer (if no space, grow buffer by alloc_by items)
void buffer_push_back(struct buffer *inst, void *item);

// Pointer to first item in buffer
void *buffer_front(struct buffer *inst);

// Pointer to last item in buffer
void *buffer_back(struct buffer *inst);

// Take last item from buffer
//  * return false if buffer is empty
//  * copy item to "out" if not null
//  * call destructor for item being removed
//  * reduce length by 1
//  * return true
bool buffer_pop(struct buffer *inst, void *out);

// Return true if buffer is empty (length == 0)
bool buffer_empty(struct buffer *inst);

// Number of items in the buffer (length)
size_t buffer_size(struct buffer *inst);

// Pointer to n'th item in buffer
void *buffer_at(struct buffer *inst, size_t idx);




//A linked-list wrapper would have a similar interface, but there would be no index/length/capacity 
//fields/functions since these operations are not trivial on a linked-list (and linked-lists don't have "capacity" since they allocate one item at a time).