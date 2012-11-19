#ifndef __SEE_VM_H__
#define __SEE_VM_H__

#include <see/config.h>
#include <see/data/object.h>
#include <see/vm/types.h>

typedef struct see_vm_exception_s  *see_vm_exception_t;
typedef struct see_vm_exception_s
{
    unsigned type;
} see_vm_exception_s;

#define SEE_VM_EXCEPTION_TYPE_ARG_ERROR       0
#define SEE_VM_EXCEPTION_TYPE_OVERFLOW        1
#define SEE_VM_EXCEPTION_TYPE_UNSUPPORTED     2
#define SEE_VM_EXCEPTION_TYPE_INVALID_PC      3
#define SEE_VM_EXCEPTION_TYPE_NO_MEM          4
#define SEE_VM_EXCEPTION_TYPE_UNHANDLED_APPLY 5

void     see_vm_run(see_vm_t vm, see_vm_exception_t exception);

#endif
