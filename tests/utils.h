#ifndef __UTILS_H
#define __UTILS_H

#include <stdlib.h>

static inline size_t min(size_t s1, size_t s2)
{
    return s1 <= s2 ? s1 : s2;
}

#endif