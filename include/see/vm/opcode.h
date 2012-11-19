#ifndef __SEE_VM_OPCODE_H__
#define __SEE_VM_OPCODE_H__

#define SEE_VM_OPCODE_ARG(op)          ((op) >> SEE_VM_OPCODE_TYPE_SHIFT)
#define SEE_VM_OPCODE_TYPE(op)         ((op) & SEE_VM_OPCODE_TYPE_MASK)
#define SEE_VM_OPCODE_TYPE_MASK        15
#define SEE_VM_OPCODE_TYPE_SHIFT       4

#define SEE_VM_OPCODE_TYPE_NO_ARG      0
#define SEE_VM_OPCODE_TYPE_CONSTANT    1
#define SEE_VM_OPCODE_TYPE_VAR         2
#define SEE_VM_OPCODE_TYPE_POP         3
#define SEE_VM_OPCODE_TYPE_CLOSURE     4
#define SEE_VM_OPCODE_TYPE_COND_JUMP   5
#define SEE_VM_OPCODE_TYPE_JUMP        6

#define SEE_VM_OPCODE_NA_NOP       0
#define SEE_VM_OPCODE_NA_IADD      1
#define SEE_VM_OPCODE_NA_ISUB      2
#define SEE_VM_OPCODE_NA_ENTER     3
#define SEE_VM_OPCODE_NA_RETURN    4
#define SEE_VM_OPCODE_NA_APPLY     5

#endif
