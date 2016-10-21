#pragma once
#include <cstd/std.h>

struct buffer {
	void *data;
	size_t item_size;
	size_t length;
	size_t capacity;
	size_t allocby;
};

void buffer_init(struct buffer *inst, size_t item_size, size_t capacity, size_t allocby);

/* Only grows buffer */
void buffer_alloc(struct buffer *inst, size_t min_capacity);

/* Will grow or truncate to the requested capacity */
void buffer_realloc(struct buffer *inst, size_t capacity);

void buffer_resize(struct buffer *inst, size_t size);

void *buffer_data(struct buffer *inst);
const void *buffer_cdata(const struct buffer *inst);

void *buffer_get(struct buffer *inst, size_t index);
const void *buffer_cget(const struct buffer *inst, size_t index);

void *buffer_head(struct buffer *inst);
void *buffer_tail(struct buffer *inst);
void *buffer_end(struct buffer *inst);
const void *buffer_chead(const struct buffer *inst);
const void *buffer_ctail(const struct buffer *inst);
const void *buffer_cend(const struct buffer *inst);

void *buffer_push(struct buffer *inst, void *in);
bool buffer_pop(struct buffer *inst, void *out);

size_t buffer_size(const struct buffer *inst);
bool buffer_empty(const struct buffer *inst);
void buffer_clear(struct buffer *inst);

void buffer_destroy(struct buffer *inst);
