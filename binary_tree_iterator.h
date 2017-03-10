#pragma once
#include <cstd/std.h>
#include "buffer.h"
#include "binary_tree.h"

struct binary_tree_iterator {
	struct buffer stack;
	bool reverse;
};

void binary_tree_iter_init(struct binary_tree_iterator *inst, struct binary_tree *tree, bool reverse);

void *binary_tree_iter_next(struct binary_tree_iterator *inst, size_t *length);

struct binary_tree_node **binary_tree_iter_next_node(struct binary_tree_iterator *inst);

void binary_tree_iter_destroy(struct binary_tree_iterator *inst);
