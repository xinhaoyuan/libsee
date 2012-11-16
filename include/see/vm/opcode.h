#ifndef __SEE_VM_OPCODE_H__
#define __SEE_VM_OPCODE_H__

#include <see/config.h>
#include <see/vm/types.h>

typedef see_vm_uword_t see_vm_opcode_t;

#define SEE_VM_OPCODE_ARG(op)          ((op) >> 4)
#define SEE_VM_OPCODE_PTR(op)          ((void *)((op) & ~(see_uintptr_t)SEE_VM_OPCODE_FLAGS_MASK))
#define SEE_VM_OPCODE_TYPE(op)         ((op) & SEE_VM_OPCODE_TYPE_MASK)
#define SEE_VM_OPCODE_TYPE_MASK        15
#define SEE_VM_OPCODE_TYPE_INLINE      0
#define SEE_VM_OPCODE_TYPE_CONSTANT    1
#define SEE_VM_OPCODE_TYPE_VAR         2
#define SEE_VM_OPCODE_TYPE_POP         3
#define SEE_VM_OPCODE_TYPE_CLOSURE     4
#define SEE_VM_OPCODE_TYPE_APPLY       5
#define SEE_VM_OPCODE_TYPE_NJUMP       6
#define SEE_VM_OPCODE_TYPE_RETURN      7

#define SEE_VM_OPCODE_INLINE_NOP       0
#define SEE_VM_OPCODE_INLINE_IADD      1
#define SEE_VM_OPCODE_INLINE_ISUB      2

#endif
