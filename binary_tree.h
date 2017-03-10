#pragma once
#include <cstd/std.h>

/*
 * Do not edit the key of a node within the tree.
 * Other data in the node which is not used by the comparator may be edited.
 */

struct binary_tree_node {
	struct binary_tree_node *children[2];
	size_t length;
	char data[];
};

typedef void *binary_tree_iterate_callback(void *arg, struct binary_tree_node *node);

typedef void binary_tree_destructor(void *data, size_t length);

typedef int binary_tree_comparator(const void *a, size_t al, const void *b, size_t bl, void *arg);

struct binary_tree {
	struct binary_tree_node *root;
	binary_tree_comparator *compare;
	binary_tree_destructor *destroy;
	void *cmparg;
	size_t size;
};

void binary_tree_init(struct binary_tree *inst, binary_tree_comparator *cmp, void *cmparg, binary_tree_destructor *destructor);

/* Number of items in the tree */
size_t binary_tree_size(struct binary_tree *inst);

/* Insert node, return existing node (without modifying) on conflict */
struct binary_tree_node **binary_tree_insert(struct binary_tree *inst, const void *data, size_t length, bool *isnew);

/* Insert node, return false on conflict */
bool binary_tree_insert_new(struct binary_tree *inst, const void *data, size_t length);

/* Insert node, delete existing if conflict (return true if conflict occurred) */
bool binary_tree_replace(struct binary_tree *inst, const void *data, size_t length);

/* Remove node if exists */
bool binary_tree_remove(struct binary_tree *inst, const void *data, size_t length);

/* Remove node from position */
bool binary_tree_delete(struct binary_tree *inst, struct binary_tree_node **node);

/* Find node data */
void *binary_tree_get(struct binary_tree *inst, const void *data, size_t length, size_t *node_length);
const void *binary_tree_cget(const struct binary_tree *inst, const void *data, size_t length, size_t *node_length);
/* Find node position */
struct binary_tree_node **binary_tree_find(struct binary_tree *inst, const void *data, size_t length);
const struct binary_tree_node *const *binary_tree_cfind(const struct binary_tree *inst, const void *data, size_t length);

/* Find minimal/maximal node data */
void *binary_tree_min(struct binary_tree *inst, size_t *node_length);
void *binary_tree_max(struct binary_tree *inst, size_t *node_length);
const void *binary_tree_cmin(const struct binary_tree *inst, size_t *node_length);
const void *binary_tree_cmax(const struct binary_tree *inst, size_t *node_length);
/* Find minimal/maximal node position */
struct binary_tree_node **binary_tree_min_node(struct binary_tree *inst);
struct binary_tree_node **binary_tree_max_node(struct binary_tree *inst);
const struct binary_tree_node *const *binary_tree_cmin_node(const struct binary_tree *inst);
const struct binary_tree_node *const *binary_tree_cmax_node(const struct binary_tree *inst);

/* Iterate over tree */
void *binary_tree_each(struct binary_tree *inst, binary_tree_iterate_callback *iter, void *arg);

/* Is tree empty? */
bool binary_tree_empty(const struct binary_tree *inst);

void binary_tree_destroy(struct binary_tree *inst);

/* Return number of bytes to compare */
typedef size_t binary_tree_default_compare_arg(void *arg, const void *data, size_t length);

/* Arg can be NULL or a binary_tree_default_compare_arg* */
int binary_tree_default_compare(const void *a, size_t al, const void *b, size_t bl, void *arg);
