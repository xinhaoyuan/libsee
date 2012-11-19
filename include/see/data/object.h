#ifndef __SEE_DATA_OBEJCT_H__
#define __SEE_DATA_OBEJCT_H__

#include <see/config.h>

typedef void *see_object_t;

/* Include the GC-specified part in object representation */
#include "gc.h"

#ifndef SEE_TYPE_ALIAS_REGISTER
#error SEE_TYPE_ALIAS_REGISTER undefined
#endif

#ifndef SEE_OBJECT_TYPE_GET
see_type_t see_object_type_get(see_object_t object);
#define SEE_OBJECT_TYPE_GET(object) (see_object_type_get(object))
#endif

#ifndef SEE_OBJECT_SIMPLE_TYPE_GET
see_simple_type_t see_object_simple_type_get(see_object_t object);
#define SEE_OBJECT_SIMPLE_TYPE_GET(object) (see_object_simple_type_get(object))
#endif

#ifndef SEE_OBJECT_TYPE_INIT
void see_object_type_init(see_object_t object, see_type_t type);
#define SEE_OBJECT_TYPE_INIT(object, type) do { see_object_type_init(object, type); } while (0)
#endif

#ifndef SEE_INTERNAL_INT_BOX
see_object_t see_internal_int_box(see_intptr_t num);
#define SEE_INTERNAL_INT_BOX(num) (see_internal_int_box(num))
#endif

#ifndef SEE_INTERNAL_INT_UNBOX
see_intptr_t see_internal_int_unbox(see_object_t object);
#define SEE_INTERNAL_INT_UNBOX(object) (see_internal_int_unbox(object))
#endif

#ifndef SEE_INTERNAL_SYMBOL_BOX
see_object_t see_internal_symbol_box(unsigned int id);
#define SEE_INTERNAL_SYMBOL_BOX(num) (see_internal_symbol_box(num))
#endif

#ifndef SEE_INTERNAL_SYMBOL_UNBOX
unsigned int see_internal_symbol_unbox(see_object_t object);
#define SEE_INTERNAL_SYMBOL_UNBOX(object) (see_internal_symbol_unbox(object))
#endif

void see_object_sys_init(void);

see_heap_t see_heap_new(void);
void       see_heap_free(see_heap_t heap);

/* heap == NULL means the object newed is managed by host code */
see_object_t see_object_new_by_size(see_heap_t heap, see_uintptr_t size);
#define SEE_OBJECT_NEW(heap, type) ((type *)(see_object_new_by_size(heap, sizeof(type))))
/* and you could only free host objects */
void         see_object_host_free(see_object_t object);

int see_object_protect(see_object_t object);
int see_object_unprotect(see_object_t object);

#endif
