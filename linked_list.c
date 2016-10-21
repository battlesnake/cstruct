#include <stdlib.h>
#include <string.h>
#include "linked_list.h"

static struct list *item_alloc(size_t length, const void *data);
static void item_remove(struct list **list, struct list *item);
static void item_insert_after(struct list **list, struct list *item);
static void item_insert_before(struct list **list, struct list *item);

/* Proxies for stateless list operators */

static void iterate_proxy(void *state, void *item)
{
	list_iterator *cb = (list_iterator *) state;
	cb(item);
}

static bool predicate_proxy(void *state, void *item)
{
	list_predicate *cb = (list_predicate *) state;
	return cb(item);
}

static void transform_proxy(void *item, struct list **out, void *state)
{
	list_transformer *cb = (list_transformer *) state;
	cb(item, out);
}

static struct list *item_alloc(size_t length, const void *data)
{
	struct list *item = malloc(sizeof(*item) + length);
	if (item == NULL) {
		exit(12);
	}
	memcpy(item->data, data, length);
	return item;
}

static void item_remove(struct list **list, struct list *item)
{
	struct list *prev = item->prev;
	struct list *next = item->next;
	prev->next = next;
	next->prev = prev;
	if (prev == item && next == item) {
		*list = NULL;
	} else if (item == *list) {
		*list = next;
	}
}

bool list_empty(const struct list *list)
{
	return list != NULL;
}

size_t list_length(const struct list *list)
{
	if (list == NULL) {
		return 0;
	}
	size_t count = 0;
	const struct list *item = list;
	do {
		count++;
		item = item->next;
	} while (item != list);
	return count;
}

static void item_insert_after(struct list **list, struct list *item)
{
	if (*list == NULL) {
		item->prev = item;
		item->next = item;
		*list = item;
	} else {
		item->prev = *list;
		item->next = (*list)->next;
		(*list)->next->prev = item;
		(*list)->next = item;
	}
}

static void item_insert_before(struct list **list, struct list *item)
{
	if (*list == NULL) {
		item->prev = item;
		item->next = item;
		*list = item;
	} else {
		item->prev = (*list)->prev;
		item->next = *list;
		(*list)->prev->next = item;
		(*list)->prev = item;
	}
}

struct list *list_insert_after(struct list **list, size_t length, const void *data)
{
	struct list *item = item_alloc(length, data);
	item_insert_after(list, item);
	return item;
}

struct list *list_insert_before(struct list **list, size_t length, const void *data)
{
	struct list *item = item_alloc(length, data);
	item_insert_before(list, item);
	return item;
}

void list_remove(struct list **list, struct list *item)
{
	item_remove(list, item);
	free(item);
}

void list_destroy(struct list **list)
{
	while (*list) {
		list_remove(list, *list);
	}
}

void list_concatenate(struct list **list, struct list **data)
{
	while (*data != NULL) {
		struct list *this = (*data);
		item_remove(data, this);
		item_insert_before(list, this);
	}
}

size_t list_each(struct list *start, list_iterator *cb)
{
	return list_each_s(start, iterate_proxy, (void *) cb);
}

size_t list_each_s(struct list *start, list_stateful_iterator *cb, void *state)
{
	if (start == NULL) {
		return 0;
	}
	struct list *item = start;
	size_t count = 0;
	do {
		cb(state, item->data);
		item = item->next;
		count++;
	} while (item != start);
	return count;
}

size_t list_filter(struct list **list, list_predicate *cb)
{
	return list_filter_s(list, predicate_proxy, (void *) cb);
}

size_t list_filter_s(struct list **list, list_stateful_predicate *cb, void *state)
{
	if (*list == NULL) {
		return 0;
	}
	struct list *item = *list;
	size_t remove_count = 0;
	bool remove;
	do {
		struct list *next = item->next;
		remove = !cb(state, item->data);
		if (remove) {
			list_remove(list, item);
			remove_count++;
		}
		item = next;
	} while (*list != NULL && (remove || item != *list));
	return remove_count;
}

struct list *list_map(struct list *start, list_transformer *cb)
{
	return list_map_s(start, transform_proxy, (void *) cb);
}

struct list *list_map_s(struct list *start, list_stateful_transformer *cb, void *state)
{
	if (start == NULL) {
		return NULL;
	}
	struct list *in = start;
	struct list *out = NULL;
	do {
		cb(in->data, &out, state);
		in = in->next;
	} while (in != start);
	return out;
}

size_t list_transform(struct list **list, list_transformer *cb)
{
	return list_transform_s(list, transform_proxy, (void *) cb);
}

size_t list_transform_s(struct list **list, list_stateful_transformer *cb, void *state)
{
	struct list *in = *list;
	struct list *out = NULL;
	size_t count = 0;
	while (*list != NULL) {
		struct list *next = in->next;
		cb(in->data, &out, state);
		list_remove(list, in);
		in = next;
		count++;
	}
	*list = out;
	return count;
}

struct list *list_first(struct list *start, list_predicate *cb)
{
	return list_first_s(start, predicate_proxy, (void *) cb);
}

struct list *list_first_s(struct list *start, list_stateful_predicate *cb, void *state)
{
	if (start == NULL) {
		return NULL;
	}
	struct list *item = start;
	do {
		if (cb(state, item->data)) {
			return item;
		}
		item = item->next;
	} while (item != start);
	return NULL;
}
