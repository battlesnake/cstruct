#pragma once
#include <stddef.h>
#include <stdbool.h>

/* Double-linked circular list, data is stored/copied into list items */

struct list
{
	struct list *prev;
	struct list *next;
	size_t length;
	char data[];
};

/* Linked list is accessed as pointer to head.  NULL if empty. */
typedef struct list *linked_list;

typedef void list_iterator(void *data);
typedef void list_stateful_iterator(void *state, void *data);

typedef bool list_predicate(void *data);
typedef bool list_stateful_predicate(void *state, void *data);

typedef void list_transformer(void *data, struct list **out);
typedef void list_stateful_transformer(void *data, struct list **out, void *state);

bool list_empty(const struct list *list);
size_t list_length(const struct list *list);
struct list *list_insert_before(struct list **list, size_t length, const void *data);
struct list *list_insert_after(struct list **list, size_t length, const void *data);
void list_remove(struct list **list, struct list *item);
void list_destroy(struct list **list);

/* Moves all items from <data> to end of <list> */
void list_concatenate(struct list **list, struct list **data);

/* Iterator over each item in the list.  *_s can can be used for reduce/fold */
size_t list_each(struct list *start, list_iterator *cb);
size_t list_each_s(struct list *start, list_stateful_iterator *cb, void *state);

/* Count for how many items the predicate returns true */
size_t list_count(struct list *start, list_predicate *cb);
size_t list_count_s(struct list *start, list_stateful_predicate *cb, void *state);

/*
 * Remove all items for which the predicate returns false, returns number of
 * items removed
  */
size_t list_filter(struct list **list, list_predicate *cb);
size_t list_filter_s(struct list **list, list_stateful_predicate *cb, void *state);

/*
 * Generate a new list by transforming the existing list, the callback uses the
 * "out" parameter to modify the output list, so many-to-one / one-to-many
 * mappings are possible
 */
struct list *list_map(struct list *start, list_transformer *cb);
struct list *list_map_s(struct list *start, list_stateful_transformer *cb, void *state);

/*
 * Like map but destroys the old list as it is processed, then replaces it with
 * the new list
 */
size_t list_transform(struct list **list, list_transformer *cb);
size_t list_transform_s(struct list **list, list_stateful_transformer *cb, void *state);

/* Returns first item for which predicate is true */
struct list *list_first(struct list *start, list_predicate *cb);
struct list *list_first_s(struct list *start, list_stateful_predicate *cb, void *state);
