#ifndef __SEE_VM_H__
#define __SEE_VM_H__

#include <data/object.h>

typedef struct see_vm_s *see_vm_t;
typedef struct see_vm_s
{
    see_vm_stack_t stack;
    see_vm_envir_t envir;
    see_vm_code_t  code;
    see_uintptr_t  pc;
    see_object_t   dump;
} see_vm_t;

#endif
