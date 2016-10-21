#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "block_alloc.h"

#define MASK_WIDTH (sizeof(size_t) * 8)
#define MASK_EMPTY ((size_t) -1)
#define MASK_FULL ((size_t) 0)
#define HIGHBIT ((size_t) 1 << (MASK_WIDTH - 1))
// For SSE4: #define MASKBIT(n) (HIGHBIT >> n)
#define MASKBIT(n) ((size_t) 1 << n)

void block_alloc_init(struct block_alloc *inst, size_t item_size)
{
	inst->item_size = item_size;
	inst->block_size = sizeof(*inst->head) + item_size * MASK_WIDTH;
	inst->head = NULL;
	inst->tail = NULL;
}

void block_alloc_destroy(struct block_alloc *inst)
{
	struct block_alloc_blk *p = inst->head;
	while (p) {
		struct block_alloc_blk *next = p->next;
		free(p);
		p = next;
	}
}

static struct block_alloc_blk *block_new(struct block_alloc *inst)
{
	struct block_alloc_blk *blk = malloc(inst->block_size);
	blk->mask = MASK_EMPTY;
	blk->next = NULL;
	if (inst->head == NULL) {
		inst->head = blk;
		blk->prev = NULL;
		inst->tail = blk;
	} else {
		inst->tail->next = blk;
		blk->prev = inst->tail;
		inst->tail = blk;
	}
	return blk;
}

static void block_delete(struct block_alloc *inst, struct block_alloc_blk *blk)
{
	if (blk == inst->head) {
		inst->head = NULL;
		inst->tail = NULL;
	} else {
		blk->prev->next = blk->next;
		blk->next->prev = blk->prev;
	}
	free(blk);
}

void *block_alloc_new(struct block_alloc *inst)
{
	struct block_alloc_blk *blk;
	if (inst->tail == NULL || inst->tail->mask == MASK_FULL) {
		blk = block_new(inst);
	} else {
		blk = inst->tail;
	}
	/* For SSE4:
	 * slot = lzcnt(blk->mask)
	 */
	int slot = 0;
	for (size_t mask = blk->mask; !(mask & 1); mask >>= 1) {
		slot++;
	}
	blk->mask ^= MASKBIT(slot);
	return blk->data + (inst->item_size * slot);
}

void block_alloc_delete(struct block_alloc *inst, void *item)
{
	const uintptr_t iitem = (uintptr_t) item;
	for (struct block_alloc_blk *p = inst->head; p; p = p->next) {
		const uintptr_t ip = (uintptr_t) p;
		if (iitem < ip + sizeof(*p)) {
			continue;
		}
		if (iitem >= ip + inst->block_size) {
		       continue;
		}	       
		const size_t slot = (iitem - (ip + sizeof(*p))) / inst->item_size;
		p->mask ^= MASKBIT(slot);
		if (p->mask == MASK_EMPTY) {
			block_delete(inst, p);
		}
		return;
	}
}
