#ifndef __SEE_XSTRING_H__
#define __SEE_XSTRING_H__

#include <see/config.h>
#include "../internal.h"

/* Simple string encapsulation for string */

typedef struct xstring_s *xstring_t;
typedef struct xstring_s
{
    see_uintptr_t ref;
    see_uintptr_t len;
    char        *cstr;
} xstring_s;

#define XSTRING_LEN_UNDEFINED ((see_uintptr_t)(-1))

xstring_t xstring_from_cstr(const char *cstr, see_uintptr_t len);
int       xstring_equal_cstr(xstring_t string, const char *cstr, see_uintptr_t len);
int       xstring_equal(xstring_t a, xstring_t b);
char     *xstring_cstr(xstring_t string);
see_uintptr_t xstring_len(xstring_t string);
void      xstring_get(xstring_t string);
void      xstring_put(xstring_t string);

#endif
