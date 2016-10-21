#include "comparator.h"

int compare_lex(const void *a, size_t al, const void *b, size_t bl)
{
	const size_t len = al > bl ? al : bl;
	const char *ap = a;
	const char *bp = b;
	for (size_t i = 0; i < len; i++, ap++, bp++) {
		if (i == al) {
			return -1;
		}
		if (i == bl) {
			return 1;
		}
		const int d = *ap - *bp;
		if (d) {
			return d;
		}
	}
	return 0;
}

int compare_lex_to(const void *a, size_t al, const void *b, size_t bl, char end)
{
	const size_t len = al > bl ? al : bl;
	const char *ap = a;
	const char *bp = b;
	for (size_t i = 0; i < len; i++, ap++, bp++) {
		if (i == al) {
			return -1;
		}
		if (i == bl) {
			return 1;
		}
		int d;
		d = (*bp == end) - (*ap == end);
		if (d) {
			return d;
		}
		d = *ap - *bp;
		if (d) {
			return d;
		}
	}
	return 0;
}
