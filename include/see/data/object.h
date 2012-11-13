#ifndef __SEE_DATA_OBEJCT_H__
#define __SEE_DATA_OBEJCT_H__

#include <see/config.h>

typedef void *see_object_t;

/* Include the GC-specified part in object representation */
#include "gc.h"

#ifndef SEE_TYPE_ALIAS_REGISTER
#error SEE_TYPE_ALIAS_REGISTER undefined
#endif

#ifndef SEE_SLOT_INIT
void see_slot_init(see_slot_t slot, see_object_t object);
#define SEE_SLOT_INIT(slot) do { see_slot_init(&(slot), object); } while (0)
#endif

#ifndef SEE_SLOT_GET
object_t see_slot_get(see_slot_t slot);
#define SEE_SLOT_GET(slot) (see_slot_get(&(slot)))
#endif

#ifndef SEE_SLOT_SET
void see_slot_set(see_slot_t slot, see_object_t object);
#define SEE_SLOT_SET(slot) do { see_slot_set(&(slot), object); } while (0)
#endif

#ifndef SEE_OBJECT_TYPE
see_type_t see_object_type(see_object_t object);
#define SEE_OBJECT_TYPE(object) (see_object_type(object))
#endif

#ifndef SEE_OBJECT_INTERNAL_TYPE
unsigned int see_object_internal_type(see_object_t object);
#define SEE_OBJECT_INTERNAL_TYPE(object) (see_object_internal_type(object))
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

typedef struct see_string_symbol_s *see_string_symbol_t;
typedef struct see_pair_s          *see_pair_t;
typedef struct see_vector_s        *see_vector_t;

typedef struct see_string_symbol_s
{
    char *string;
} see_string_symbol_s;

typedef struct see_pair_s
{
    see_slot_s car, cdr;
} see_pair_s;

typedef struct see_vector_s
{
    see_uintptr_t length;
    see_slot_s   *slot_entry;
} see_vector_s;

/* For access without type checking and bound checking */
#define SEE_PAIR_CAR(object)                 SEE_SLOT_GET(((see_pair_t)(object))->car)
#define SEE_PAIR_CDR(object)                 SEE_SLOT_GET(((see_pair_t)(object))->cdr)
#define SEE_PAIR_CAR_SET(object, value)      SEE_SLOT_SET(((see_pair_t)(object))->car, value)
#define SEE_PAIR_CDR_SET(object, value)      SEE_SLOT_SET(((see_pair_t)(object))->cdr, value)
#define SEE_VECTOR_LEN(object)               (((see_vector_t)(object))->length)
#define SEE_VECTOR_REF(object, index)        SEE_SLOT_GET(((see_vector_t)(object))->slot_entry[index])
#define SEE_VECTOR_SET(object, index, value) SEE_SLOT_SET(((see_vector_t)(object))->slot_entry[index], value)
#define SEE_STRING_SYMBOL_CSTR(object)       (((see_string_symbol_t)(object))->string)

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
