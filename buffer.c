#include "buffer.h"

void buffer_init(struct buffer *inst, size_t item_size, size_t capacity, size_t allocby)
{
	inst->data = NULL;
	inst->item_size = item_size;
	inst->length = 0;
	inst->capacity = 0;
	inst->allocby = allocby;
	buffer_alloc(inst, capacity);
}

void buffer_alloc(struct buffer *inst, size_t min_capacity)
{
	if (min_capacity <= inst->capacity) {
		return;
	}
	buffer_realloc(inst, min_capacity);
}

void buffer_realloc(struct buffer *inst, size_t capacity)
{
	if (inst->length > capacity) {
		inst->length = inst->capacity;
	}
	inst->data = realloc(inst->data, inst->item_size * capacity);
	inst->capacity = capacity;
}

void buffer_resize(struct buffer *inst, size_t size)
{
	buffer_alloc(inst, size);
	inst->length = size;
}

void *buffer_data(struct buffer *inst)
{
	return inst->data;
}

const void *buffer_cdata(const struct buffer *inst)
{
	return inst->data;
}

void *buffer_ptr(struct buffer *inst, size_t index)
{
	return (void *) ((char *) inst->data + index * inst->item_size);
}

const void *buffer_cptr(const struct buffer *inst, size_t index)
{
	return buffer_ptr((struct buffer *)inst, index);
}

void *buffer_get(struct buffer *inst, size_t index)
{
	if (index >= inst->length) {
		return NULL;
	}
	return buffer_ptr(inst, index);
}

const void *buffer_rget(const struct buffer *inst, size_t index)
{
	if (index >= inst->length) {
		return NULL;
	}
	return buffer_cptr(inst, index);
}

void buffer_clear(struct buffer *inst)
{
	buffer_resize(inst, 0);
}

size_t buffer_size(const struct buffer *inst)
{
	return inst->length;
}

bool buffer_empty(const struct buffer *inst)
{
	return inst->length == 0;
}

void buffer_destroy(struct buffer *inst)
{
	free(inst->data);
}

void *buffer_head(struct buffer *inst)
{
	if (inst->length == 0) {
		return NULL;
	}
	return buffer_get(inst, 0);
}

void *buffer_tail(struct buffer *inst)
{
	if (inst->length == 0) {
		return NULL;
	}
	return buffer_get(inst, inst->length - 1);
}

void *buffer_end(struct buffer *inst)
{
	return (void *) ((char *) inst->data + inst->length * inst->item_size);
}

const void *buffer_chead(const struct buffer *inst)
{
	return buffer_head((struct buffer *) inst);
}

const void *buffer_ctail(const struct buffer *inst)
{
	return buffer_tail((struct buffer *) inst);
}

const void *buffer_cend(const struct buffer *inst)
{
	return (const void *) ((const char *) inst->data + inst->length * inst->item_size);
}

void *buffer_push(struct buffer *inst, void *in)
{
	size_t i = inst->length;
	if (i == inst->capacity) {
		buffer_alloc(inst, i + inst->allocby * inst->item_size);
	}
	++inst->length;
	void *p = buffer_get(inst, i);
	if (in) {
		memcpy(p, in, inst->item_size);
	}
	return p;
}

bool buffer_pop(struct buffer *inst, void *out)
{
	if (inst->length == 0) {
		return false;
	}
	if (out) {
		memcpy(out, buffer_ctail(inst), inst->item_size);
	}
	buffer_resize(inst, inst->length - 1);
	return true;
}
