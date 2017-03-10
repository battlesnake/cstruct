#if 0
(
set -euo pipefail
declare -r tmp="$(mktemp)"
gcc -I./c_modules -Wall -Wextra -Werror -std=c11 -O0 -g -DTEST_binary_tree -o "$tmp" *.c
exec valgrind --quiet --leak-check=full --track-origins=yes "$tmp"
)
exit 0
#endif
#include <cstd/std.h>
#include "comparator.h"
#include "binary_tree.h"

typedef const struct binary_tree_node *const * node_clocation;

/* TODO: red-black balancing */

int binary_tree_default_compare(const void *a, size_t al, const void *b, size_t bl, void *arg)
{
	if (arg) {
		binary_tree_default_compare_arg *f = arg;
		al = f(arg, a, al);
		bl = f(arg, b, bl);
	}
	return compare_lex(a, al, b, bl);
}

void binary_tree_init(struct binary_tree *inst, binary_tree_comparator *cmp, void *cmparg, binary_tree_destructor *destructor)
{
	inst->root = NULL;
	inst->compare = cmp ? cmp : binary_tree_default_compare;
	inst->cmparg = cmparg;
	inst->destroy = destructor;
	inst->size = 0;
}

size_t binary_tree_size(struct binary_tree *inst)
{
	return inst->size;
}

static void *node_data(struct binary_tree_node **pos, size_t *length)
{
	if (*pos == NULL) {
		if (length != NULL) {
			*length = 0;
		}
		return NULL;
	}
	if (length != NULL) {
		*length = (*pos)->length;
	}
	return (*pos)->data;
}

static const void *node_cdata(const node_clocation pos, size_t *length)
{
	return node_data((struct binary_tree_node **) pos, length);
}

static void set_node(struct binary_tree *inst, struct binary_tree_node **pos, struct binary_tree_node *node)
{
	*pos = node;
	inst->size++;
}

static struct binary_tree_node *unset_node(struct binary_tree *inst, struct binary_tree_node **pos)
{
	if (pos == NULL) {
		return NULL;
	}
	struct binary_tree_node *node = *pos;
	*pos = NULL;
	if (node) {
		inst->size--;
	}
	return node;
}

static struct binary_tree_node *do_create(const void *data, size_t length)
{
	struct binary_tree_node *node = malloc(sizeof(*node) + length);
	memset(node->children, 0, sizeof(node->children));
	node->length = length;
	memcpy(node->data, data, length);
	return node;
}

static void do_destroy(struct binary_tree *inst, struct binary_tree_node **pos)
{
	struct binary_tree_node *node = unset_node(inst, pos);
	if (node == NULL) {
		return;
	}
	if (inst->destroy) {
		inst->destroy(node->data, node->length);
	}
	free(node);
}

static bool do_insert(struct binary_tree *inst, struct binary_tree_node *node)
{
	if (node == NULL) {
		return false;
	}
	struct binary_tree_node **pos = binary_tree_find(inst, node->data, node->length);
	bool is_new = *pos == NULL;
	if (is_new) {
		set_node(inst, pos, node);
	}
	return is_new;
}

static bool prune(struct binary_tree *inst, struct binary_tree_node **pos)
{
	(void) inst;
	if (*pos == NULL) {
		return false;
	}
	prune(inst, &(*pos)->children[0]);
	prune(inst, &(*pos)->children[1]);
	do_destroy(inst, pos);
	return true;
}

void binary_tree_clear(struct binary_tree *inst)
{
	prune(inst, &inst->root);
}

struct binary_tree_node **binary_tree_insert(struct binary_tree *inst, const void *data, size_t length, bool *isnew)
{
	struct binary_tree_node **pos = binary_tree_find(inst, data, length);
	bool is_new = *pos == NULL;
	if (isnew) {
		*isnew = is_new;
	}
	if (is_new) {
		set_node(inst, pos, do_create(data, length));
	}
	return pos;
}

bool binary_tree_insert_new(struct binary_tree *inst, const void *data, size_t length)
{
	bool isnew;
	binary_tree_insert(inst, data, length, &isnew);
	return isnew;
}

bool binary_tree_replace(struct binary_tree *inst, const void *data, size_t length)
{
	bool isnew;
	struct binary_tree_node **p = binary_tree_insert(inst, data, length, &isnew);
	if (isnew) {
		return false;
	}
	struct binary_tree_node *children[] = {
		unset_node(inst, &(*p)->children[0]),
		unset_node(inst, &(*p)->children[1])
	};
	binary_tree_delete(inst, p);
	set_node(inst, p, do_create(data, length));
	set_node(inst, &(*p)->children[0], children[0]);
	set_node(inst, &(*p)->children[1], children[1]);
	return true;
}

bool binary_tree_remove(struct binary_tree *inst, const void *data, size_t length)
{
	return binary_tree_delete(inst, binary_tree_find(inst, data, length));
}

bool binary_tree_delete(struct binary_tree *inst, struct binary_tree_node **node)
{
	(void) inst;
	if (*node == NULL) {
		return false;
	}
	struct binary_tree_node *n = *node;
	struct binary_tree_node *children[] = {
		unset_node(inst, &n->children[0]),
		unset_node(inst, &n->children[1])
	};
	do_destroy(inst, node);
	do_insert(inst, children[0]);
	do_insert(inst, children[1]);
	return true;
}

void *binary_tree_get(struct binary_tree *inst, const void *data, size_t length, size_t *node_length)
{
	return node_data(binary_tree_find(inst, data, length), node_length);
}

const void *binary_tree_cget(const struct binary_tree *inst, const void *data, size_t length, size_t *node_length)
{
	return binary_tree_get((struct binary_tree *) inst, data, length, node_length);
}

struct binary_tree_node **binary_tree_find(struct binary_tree *inst, const void *data, size_t length)
{
	struct binary_tree_node **p = &inst->root;
	while (*p) {
		int c = inst->compare(data, length, (*p)->data, (*p)->length, inst->cmparg);
		if (c == 0) {
			return p;
		}
		p = &(*p)->children[c > 0 ? 1 : 0];
	}
	return p;
}

node_clocation binary_tree_cfind(const struct binary_tree *inst, const void *data, size_t length)
{
	return (node_clocation) binary_tree_find((struct binary_tree *) inst, data, length);
}

struct recurse_closure {
	binary_tree_iterate_callback *iter;
	void *arg;
};

static void *recurse_iter(struct recurse_closure *arg, struct binary_tree_node *node)
{
	if (node == NULL) {
		return NULL;
	}
	void *res;
	res = recurse_iter(arg, node->children[0]);
	if (res) {
		return res;
	}
	res = arg->iter(arg->arg, node);
	if (res) {
		return res;
	}
	res = recurse_iter(arg, node->children[1]);
	if (res) {
		return res;
	}
	return NULL;
}

void *binary_tree_each(struct binary_tree *inst, binary_tree_iterate_callback *iter, void *arg)
{
	struct recurse_closure rc = {
		.iter = iter,
		.arg = arg
	};
	return recurse_iter(&rc, inst->root);
}

void binary_tree_destroy(struct binary_tree *inst)
{
	prune(inst, &inst->root);
}

bool binary_tree_empty(const struct binary_tree *inst)
{
	return inst->root == NULL;
}

/* minimal/maximal */

void *binary_tree_min(struct binary_tree *inst, size_t *node_length)
{
	return node_data(binary_tree_min_node(inst), node_length);
}

void *binary_tree_max(struct binary_tree *inst, size_t *node_length)
{
	return node_data(binary_tree_max_node(inst), node_length);
}

const void *binary_tree_cmin(const struct binary_tree *inst, size_t *node_length)
{
	return node_cdata(binary_tree_cmin_node(inst), node_length);
}

const void *binary_tree_cmax(const struct binary_tree *inst, size_t *node_length)
{
	return node_cdata(binary_tree_cmax_node(inst), node_length);
}

struct binary_tree_node **binary_tree_min_node(struct binary_tree *inst)
{
	struct binary_tree_node **prev = NULL;
	for (struct binary_tree_node **n = &inst->root; *n; n = &(*n)->children[0]) {
		prev = n;
	}
	return prev;
}

struct binary_tree_node **binary_tree_max_node(struct binary_tree *inst)
{
	struct binary_tree_node **prev = NULL;
	for (struct binary_tree_node **n = &inst->root; *n; n = &(*n)->children[1]) {
		prev = n;
	}
	return prev;
}

node_clocation binary_tree_cmin_node(const struct binary_tree *inst)
{
	return (node_clocation) binary_tree_min_node((struct binary_tree *) inst);
}

node_clocation binary_tree_cmax_node(const struct binary_tree *inst)
{
	return (node_clocation) binary_tree_max_node((struct binary_tree *) inst);
}

/* Test */

#if defined TEST_binary_tree

static size_t cmpkv(void *arg, const void *data, size_t length)
{
	(void) arg;
	for (size_t i = 0; i < length; i++) {
		if (((char *) data)[i] == '=') {
			return i;
		}
	}
	return length;
}

static struct binary_tree_node **add_str(struct binary_tree *tree, const char *s)
{
	bool isnew;
	struct binary_tree_node **res = binary_tree_insert(tree, s, strlen(s) + 1, &isnew);
	if (!isnew) {
		printf(" * Duplicate rejected: '%s' conflicts with '%s'\n", s, (*res)->data);
	}
	return res;
}

static void replace_str(struct binary_tree *tree, const char *s)
{
	binary_tree_replace(tree, s, strlen(s) + 1);
}

static void *print_str(void *arg, struct binary_tree_node *node)
{
	printf("%s%.*s\n", (char *) arg, (int) node->length, (char *) node->data);
	return NULL;
}

static void test_destroy(void *data, size_t length)
{
	printf("   (Destroying node: %.*s)\n", (int) length, (char *) data);
}

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
	struct binary_tree tree;
	binary_tree_init(&tree, NULL, cmpkv, test_destroy);

	printf("Building tree\n");
	struct binary_tree_node **n = add_str(&tree, "key=value");
	add_str(&tree, "another key=value");
	add_str(&tree, "another key=another value");
	add_str(&tree, "more keys=more values");
	add_str(&tree, "more keys=even more values");
	printf("\n");

	printf("Querying tree\n");
	printf(" * Minimum node: %s\n", (char *) binary_tree_min(&tree, NULL));
	printf(" * Maximum node: %s\n", (char *) binary_tree_max(&tree, NULL));
	printf("\n");

	printf("Listing tree\n");
	binary_tree_each(&tree, print_str, " * ");
	printf("\n");

	printf("Replacing first-added node\n");
	replace_str(&tree, "key=new value");
	printf("\n");

	printf("Listing tree after replacement\n");
	binary_tree_each(&tree, print_str, " * ");
	printf("\n");

	printf("Deleting first-added node\n");
	binary_tree_delete(&tree, n);
	printf("\n");

	printf("Listing tree after deletion\n");
	binary_tree_each(&tree, print_str, " * ");
	printf("\n");

	printf("Destroying tree\n");
	binary_tree_destroy(&tree);
	printf("\n");
	return 0;
}
#endif
