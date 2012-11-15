#ifndef __SEE_DATA_GC_H__
#define __SEE_DATA_GC_H__

typedef struct see_slot_s      *see_slot_t;
typedef struct see_heap_s      *see_heap_t;
typedef struct see_gc_header_s *see_gc_header_t;
typedef struct see_type_s      *see_type_t;

#define SEE_OBJECT_TYPE_NULL             ((see_type_t)0x00)
#define SEE_OBJECT_TYPE_INTERNAL_INT     ((see_type_t)0x01)
#define SEE_OBJECT_TYPE_INTERNAL_SYMBOL  ((see_type_t)0x02)
#define SEE_OBJECT_TYPE_INTERNAL_TOTAL   0x8
#define SEE_OBJECT_TYPE_ERROR            ((see_type_t)0x08)
#define SEE_OBJECT_TYPE_DUMMY            ((see_type_t)0x09)
#define SEE_OBJECT_TYPE_STRING_SYMBOL    ((see_type_t)0x0a)
#define SEE_OBJECT_TYPE_PAIR             ((see_type_t)0x0b)
#define SEE_OBJECT_TYPE_VECTOR           ((see_type_t)0x0c)

#define SEE_OBJECT_TYPE_USER_DEFINE_ID   0x0d

#define SEE_OBJECT_TYPE_EXTERNAL         ((see_type_t)0xff)
#define SEE_OBJECT_TYPE_TOTAL            0x100

/* SEE assumes that the malloc func is aligned to 8, thus leaving last
 * 3 bits of a pointer free for atomic data type masking */

/* The mask pattern is like this: */
/* XX1   -- Boxed integer         */
/* 010   -- Internal symbol       */
/* Other -- Undefined             */
#define SEE_OBJECT_TYPE_MASK 0x7
#define SEE_IS_INTERNAL_INT(obj)       ((see_uintptr_t)(obj) & 1)
#define SEE_IS_INTERNAL_SYMBOL(obj)    (((see_uintptr_t)(obj) & 0x7) == SEE_OBJECT_TYPE_INTERNAL_SYMBOL)
#define SEE_IS_INTERNAL_CLOSURE(obj)   (((see_uintptr_t)(obj) & 0x7) == SEE_OBJECT_TYPE_INTERNAL_CLOSURE)
#define SEE_INTERNAL_INT_BOX(num)      ((see_object_t)(((see_uintptr_t)(num) << 1) | 1))
#define SEE_INTERNAL_INT_UNBOX(obj)    ((see_intptr_t)(obj) >> 1)
#define SEE_INTERNAL_SYMBOL_BOX(id)    ((see_object_t)(((see_uintptr_t)(id) << 3) | (see_uintptr_t)SEE_OBJECT_TYPE_INTERNAL_SYMBOL))
#define SEE_INTERNAL_SYMBOL_UNBOX(obj) ((see_uintptr_t)(obj) >> 3)

#define SEE_SYMBOL_ID_NULL             0
#define SEE_SYMBOL_ID_TRUE             1
#define SEE_SYMBOL_ID_FALSE            2
#define SEE_SYMBOL_ID_USER_DEFINE      3

#define SEE_OBJECT_NULL                SEE_INTERNAL_SYMBOL_BOX(SEE_SYMBOL_ID_NULL)
#define SEE_OBJECT_TRUE                SEE_INTERNAL_SYMBOL_BOX(SEE_SYMBOL_ID_TRUE)
#define SEE_OBJECT_FALSE               SEE_INTERNAL_SYMBOL_BOX(SEE_SYMBOL_ID_FALSE)

typedef struct see_slot_s
{
    see_object_t __value;
} see_slot_s;

#define SEE_SLOT_GET(slot)         ((slot).__value)
#define SEE_SLOT_SET(slot, value)  do { (slot).__value = (value); } while (0)
#define SEE_SLOT_INIT(slot, value) do { (slot).__value = (value); } while (0)

typedef struct see_heap_s
{
    see_uintptr_t    object_count;
    see_uintptr_t    gc_threshold;
    
    see_object_t *tracker;
    int           error_flag;
} see_heap_s;

typedef struct see_gc_header_s
{
    union{
        struct
        {
            unsigned char  type;
            unsigned char  gc_mark;
            unsigned short protect_level;
        };

        char __space_filler[8];
    };
} see_gc_header_s;

#define SEE_PROTECT_LEVEL_MAX 0xffff

static const char __static_assumption_for_gc_header_size[sizeof(see_gc_header_s) % 8 == 0 ? 0 : -1];

typedef struct see_type_s
{
    const char*(*name)(see_type_t self, see_object_t object);
    void(*enumerate)(see_type_t self, see_object_t object, void(*touch)(void *priv, see_object_t data), void *priv);
    void(*free)(see_type_t self, see_object_t object);
} see_type_s;

extern see_type_t see_types[SEE_OBJECT_TYPE_TOTAL];
static inline void see_type_alias_register(see_type_t type, see_type_t interface)
{
    see_types[(see_uintptr_t)type] = interface;
}

#define SEE_TYPE_ALIAS_REGISTER(type, interface) do { see_type_alias_register(type, interface); } while (0)

#endif
