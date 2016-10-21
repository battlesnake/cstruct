#pragma once
#include <stddef.h>

struct block_alloc_blk {
	size_t mask;
	struct block_alloc_blk *prev;
	struct block_alloc_blk *next;
	char data[];
};

struct block_alloc {
	/* Start/end of list */
	struct block_alloc_blk *head;
	struct block_alloc_blk *tail;
	/* Size of one item */
	size_t item_size;
	/* Size of block */
	size_t block_size;
};

void block_alloc_init(struct block_alloc *inst, size_t item_size);
void block_alloc_destroy(struct block_alloc *inst);

void *block_alloc_new(struct block_alloc *inst);
void block_alloc_delete(struct block_alloc *inst, void *item);
