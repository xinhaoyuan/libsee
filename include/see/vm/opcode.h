#ifndef __SEE_VM_OPCODE_H__
#define __SEE_VM_OPCODE_H__

#include <see/config.h>

typedef see_uintptr_t see_vm_opcode_t;

#define OPCODE_ARG(op)          ((op) >> 4)
#define OPCODE_PTR(op)          ((void *)((op) & ~(see_uintptr_t)OPCODE_FLAGS_MASK))
#define OPCODE_TYPE(op)         ((op) & OPCODE_TYPE_MASK))
#define OPCODE_TYPE_MASK        15
#define OPCODE_TYPE_COMPOUND    0
#define OPCODE_TYPE_VAR         1
#define OPCODE_TYPE_POP         2
#define OPCODE_TYPE_CLOSURE     3
#define OPCODE_TYPE_APPLY       4
#define OPCODE_TYPE_NJUMP       5
#define OPCODE_TYPE_INLINE      6

#define OPCODE_INLINE_NOP       0

#endif
