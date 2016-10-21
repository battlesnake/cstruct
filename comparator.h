#pragma once
#include <stddef.h>

/* Lexicographical comparison */
int compare_lex(const void *a, size_t al, const void *b, size_t bl);

/* Lexicographical comparison up to (but not including) given char */
int compare_lex_to(const void *a, size_t al, const void *b, size_t bl, char end);
