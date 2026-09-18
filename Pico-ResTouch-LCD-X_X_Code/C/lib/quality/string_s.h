#ifndef STRING_S_H
#define STRING_S_H

/*
 * C11 Annex K memset_s. glibc and Pico newlib do not provide it;
 * clang-tidy still wants the call sites to use this API.
 */
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int errno_t;
typedef size_t rsize_t;

errno_t memset_s(void *s, rsize_t smax, int c, rsize_t n);

#ifdef __cplusplus
}
#endif

#endif
