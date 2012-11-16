#ifndef __SEE_CONFIG_H__
#define __SEE_CONFIG_H__

/* 1 = using standard headers and routines */
#define USING_STD 1

#if USING_STD

/* Providing pointer-size integer types by stdint.h */
#include <stdint.h>
typedef uintptr_t see_uintptr_t;
typedef intptr_t  see_intptr_t;
typedef uint64_t  see_uint64_t;
typedef int64_t   see_int64_t;
typedef uint32_t  see_uint32_t;
typedef int32_t   see_int32_t;
#define SEE_PTR_BITS (sizeof(see_uintptr_t) * 8)
/* And memory/string routines from stdlib.h/string.h */
#include <stdlib.h>
#include <string.h>
#define SEE_MALLOC  malloc
#define SEE_FREE    free
#define SEE_REALLOC realloc
#define SEE_MEMCPY  memcpy
#define SEE_STRLEN  strlen
#define SEE_MEMCMP  memcmp
#define SEE_DEBUG_ERROR(v ...)

#include <stdio.h>
#define USING_FILE 1
#define SEE_FILE_T  FILE *
#define SEE_FPRINTF fprintf
#define SEE_INTPTR_FD "%ld"

#else

/* Or you define those stuffs by yourself */

#endif

#endif
