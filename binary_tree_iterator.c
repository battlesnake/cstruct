#if 0
(
set -euo pipefail
declare -r tmp="$(mktemp)"
gcc -I./c_modules -Wall -Wextra -Werror -std=c11 -O0 -g -DTEST_binary_tree_iterator -o "$tmp" *.c
exec valgrind --quiet --leak-check=full --track-origins=yes "$tmp"
)
exit 0
#endif
#include <cstd/std.h>
#include "binary_tree_iterator.h"

static void enter(struct binary_tree_iterator *inst, struct binary_tree_node *node)
{
	for (; node; node = node->children[inst->reverse ? 1 : 0]) {
		buffer_push(&inst->stack, &node);
	}
}

void binary_tree_iter_init(struct binary_tree_iterator *inst, struct binary_tree *tree, bool reverse)
{
	inst->reverse = reverse;
	buffer_init(&inst->stack, sizeof(struct binary_tree_node *), 64, 64);
	enter(inst, tree->root);
}

struct binary_tree_node *binary_tree_iter_next_node(struct binary_tree_iterator *inst)
{
	struct binary_tree_node *node;
	if (!buffer_pop(&inst->stack, &node)) {
		return NULL;
	}
	enter(inst, node->children[inst->reverse ? 0 : 1]);
	return node;
}

void *binary_tree_iter_next(struct binary_tree_iterator *inst, size_t *length)
{
	struct binary_tree_node *node = binary_tree_iter_next_node(inst);
	if (node == NULL) {
		return NULL;
	}
	if (length != NULL) {
		*length = node->length;
	}
	return node->data;
}

void binary_tree_iter_destroy(struct binary_tree_iterator *inst)
{
	buffer_destroy(&inst->stack);
}

#if defined TEST_binary_tree_iterator
static void *print_num(void *arg, struct binary_tree_node *node)
{
	(void) arg;
	int x = *(const int *) node->data;
	printf(" * %d\n", x);
	return NULL;
}

static int cmpi(const void *a, size_t al, const void *b, size_t bl, void *arg)
{
	(void) al;
	(void) bl;
	(void) arg;
	return *(const int *) a - *(const int *) b;
}

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
	struct binary_tree tree;
	const int data[] = { 100, 110, 120, 130, 80, 150, 10, 140, 200, 40, 30, 20 };

	binary_tree_init(&tree, cmpi, NULL, NULL);

	for (size_t i = 0; i < sizeof(data)/sizeof(data[0]); i++) {
		binary_tree_insert_new(&tree, &data[i], sizeof(data[i]));
	}

	printf("Tree initialised to:\n");
	binary_tree_each(&tree, print_num, NULL);
	printf("\n");

	printf("Minimum value: %d\n", *(const int *) binary_tree_cmin(&tree, NULL));
	printf("Maximum value: %d\n", *(const int *) binary_tree_cmax(&tree, NULL));
	printf("\n");

	struct binary_tree_iterator it;
	const int *p;

	printf("Iterator sequence (forward):\n");
	binary_tree_iter_init(&it, &tree, false);
	while ((p = (const int *) binary_tree_iter_next(&it, NULL))) {
		printf(" * %d\n", *p);
	}
	binary_tree_iter_destroy(&it);
	printf("\n");

	printf("Iterator sequence (reverse):\n");
	binary_tree_iter_init(&it, &tree, true);
	while ((p = (const int *) binary_tree_iter_next(&it, NULL))) {
		printf(" * %d\n", *p);
	}
	binary_tree_iter_destroy(&it);
	printf("\n");

	binary_tree_destroy(&tree);
	return 0;
}
#endif
