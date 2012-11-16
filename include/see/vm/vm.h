#ifndef __SEE_VM_H__
#define __SEE_VM_H__

#include <see/config.h>
#include <see/data/object.h>
#include <see/vm/types.h>
#include <see/vm/prog.h>

typedef struct see_vm_s           *see_vm_t;
typedef struct see_vm_stack_s     *see_vm_stack_t;
typedef struct see_vm_envir_s     *see_vm_envir_t;
typedef struct see_vm_exception_s *see_vm_exception_t;

typedef struct see_vm_stack_s
{
    see_uintptr_t size;
    see_uintptr_t allocated;
    see_object_t *entry;
} see_vm_stack_s;

typedef struct see_vm_envir_s
{
    see_vm_envir_t parent;
    see_vm_uword_t size;
    see_slot_s    *slots;
} see_vm_envir_s;

typedef struct see_vm_exception_s
{
    unsigned type;
} see_vm_exception_s;

#define SEE_VM_EXCEPTION_TYPE_ARG_ERROR   0
#define SEE_VM_EXCEPTION_TYPE_OVERFLOW    1
#define SEE_VM_EXCEPTION_TYPE_UNSUPPORTED 2
#define SEE_VM_EXCEPTION_TYPE_INVALID_PC  3

typedef struct see_vm_s
{
    see_heap_t     heap;
    see_vm_stack_s stack;
    see_vm_envir_t envir;
    see_vm_prog_t  prog;
    see_vm_uword_t pc;
    see_object_t   dump;
} see_vm_s;


#define SEE_VM_EXCEPTION_TYPE_CALL 0

see_vm_t see_vm_new(see_heap_t heap, see_vm_prog_t prog);
int      see_vm_push(see_vm_t vm, see_object_t data);
int      see_vm_pop(see_vm_t vm, see_uintptr_t size);
void     see_vm_run(see_vm_t vm, see_vm_exception_t exception);

#endif
