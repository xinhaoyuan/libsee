#ifndef __SEE_VM_TYPES_H__
#define __SEE_VM_TYPES_H__

#include <see/config.h>
#include <see/data/object.h>
#include <see/vm/opcode.h>
#include <see/io/char_stream.h>

typedef see_uint64_t   see_vm_uword_t;
typedef see_int64_t    see_vm_word_t;
typedef see_vm_uword_t see_vm_opcode_t;

typedef struct see_vm_slot_s *see_vm_slot_t;
typedef struct see_vm_slot_s
{
    unsigned type;
    union
    {
        see_object_t   _object;
        see_vm_word_t  _word;
        see_vm_uword_t _uword;
        void          *_ptr;
    };
} see_vm_slot_s;

#define SEE_VM_SLOT_TYPE_OBJECT 0
#define SEE_VM_SLOT_TYPE_WORD   1
#define SEE_VM_SLOT_TYPE_UWORD  2
#define SEE_VM_SLOT_TYPE_PTR    3

typedef struct see_vm_string_symbol_s *see_vm_string_symbol_t;
typedef struct see_vm_pair_s          *see_vm_pair_t;
typedef struct see_vm_vector_s        *see_vm_vector_t;
typedef struct see_vm_closure_s       *see_vm_closure_t;
typedef struct see_vm_prog_s          *see_vm_prog_t;
typedef struct see_vm_envir_s         *see_vm_envir_t;
typedef struct see_vm_s               *see_vm_t;
typedef struct see_vm_stack_s         *see_vm_stack_t;

typedef struct see_vm_string_symbol_s
{
    char *string;
} see_vm_string_symbol_s;

typedef struct see_vm_pair_s
{
    see_vm_slot_s car, cdr;
} see_vm_pair_s;

typedef struct see_vm_vector_s
{
    see_vm_uword_t length;
    see_vm_slot_s *entry;
} see_vm_vector_s;

typedef struct see_vm_closure_s
{
    see_vm_envir_t envir;
    see_vm_prog_t  prog;
    see_vm_uword_t entry;
} see_vm_closure_s;

typedef struct see_vm_prog_s
{
    see_vm_uword_t   constant_count;
    see_vm_slot_s   *constant_slot;
    see_vm_uword_t   opcode_count;
    see_vm_opcode_t *opcode_entry;
} see_vm_prog_s;

typedef struct see_vm_envir_s
{
    see_vm_envir_t parent;
    see_vm_uword_t size;
    see_vm_slot_s *slot;
} see_vm_envir_s;

typedef struct see_vm_stack_s
{
    see_uintptr_t  size;
    see_uintptr_t  bound;
    see_uintptr_t  allocated;
    see_vm_slot_s *entry;
} see_vm_stack_s;

typedef struct see_vm_s
{
    see_heap_t     heap;
    see_vm_stack_s stack;
    see_vm_envir_t envir;
    see_vm_prog_t  prog;
    see_vm_uword_t pc;
    see_object_t   dump;
} see_vm_s;

#define SEE_TYPE_VM_STRING_SYMBOL ((see_type_t)(SEE_TYPE_USER_DEFINE_START + 0))
#define SEE_TYPE_VM_PAIR          ((see_type_t)(SEE_TYPE_USER_DEFINE_START + 1))
#define SEE_TYPE_VM_VECTOR        ((see_type_t)(SEE_TYPE_USER_DEFINE_START + 2))
#define SEE_TYPE_VM_CLOSURE       ((see_type_t)(SEE_TYPE_USER_DEFINE_START + 3))
#define SEE_TYPE_VM_PROG          ((see_type_t)(SEE_TYPE_USER_DEFINE_START + 4))
#define SEE_TYPE_VM_ENVIR         ((see_type_t)(SEE_TYPE_USER_DEFINE_START + 5))
#define SEE_TYPE_VM               ((see_type_t)(SEE_TYPE_USER_DEFINE_START + 6))

void see_vm_types_sys_init(void);
/* parse binary input stream */
see_vm_prog_t see_vm_prog_parse_bin(see_heap_t heap, see_io_char_stream_in_f in, void *data);
see_vm_closure_t see_vm_closure_new(see_heap_t heap, see_vm_envir_t envir, see_vm_prog_t prog, see_vm_uword_t entry);
see_vm_t see_vm_new(see_heap_t heap, see_vm_prog_t prog);

#endif
