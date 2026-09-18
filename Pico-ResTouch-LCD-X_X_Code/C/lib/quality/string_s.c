#include "string_s.h"

errno_t memset_s(void *s, rsize_t smax, int c, rsize_t n)
{
    volatile unsigned char *p;
    rsize_t i;

    if (s == NULL || n > smax) {
        return -1;
    }
    p = (volatile unsigned char *)s;
    for (i = 0; i < n; i++) {
        p[i] = (unsigned char)c;
    }
    return 0;
}
