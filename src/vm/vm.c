#include <see/vm/vm.h>

see_vm_t
see_vm_new(see_heap_t heap, see_vm_prog_t prog)
{
    see_vm_t vm = SEE_OBJECT_NEW(heap, see_vm_s);
    if (vm == NULL) return NULL;

    vm->stack.allocated = 0;
    vm->stack.size      = 0;
    vm->stack.entry     = NULL;
    
    vm->envir = NULL;
    vm->prog  = prog;
    vm->pc    = 0;
    vm->dump  = NULL;

    return vm;
}

#define VM_PUSH(entry, size, allocated, data) ({                        \
            if (size == allocated)                                      \
            {                                                           \
                see_uintptr_t na;                                       \
                if (allocated)                                          \
                    na = allocated << 1;                                \
                else na = 64;                                           \
                see_object_t *ne = SEE_REALLOC(entry, na * sizeof(see_object_t)); \
                allocated = na;                                         \
                entry = ne;                                             \
            }                                                           \
            entry == NULL ? -1 : (entry[size ++] = data, 0);            \
        })

#define VM_COMPACT(entry, size, allocated) do {                 \
        see_uintptr_t na = allocated;                           \
        while (size < (na >> 1))                                \
            na >>= 1;                                           \
        allocated = na;                                         \
        entry = SEE_REALLOC(entry, na * sizeof(see_object_t));  \
    } while (0)

int
see_vm_push(see_vm_t vm, see_object_t data)
{
    return VM_PUSH(vm->stack.entry, vm->stack.size, vm->stack.allocated, data);
}

int
see_vm_pop(see_vm_t vm, see_uintptr_t size)
{
    if (vm->stack.size < size) return -1;
    
    vm->stack.size -= size;
    VM_COMPACT(vm->stack.entry, vm->stack.size, vm->stack.allocated);
    return 0;
}

void
see_vm_run(see_vm_t vm, see_vm_exception_t exception)
{
    see_vm_prog_t            prog = vm->prog;
    see_vm_uword_t   opcode_count = prog->opcode_count;
    see_vm_opcode_t *opcode_entry = prog->opcode_entry;
    see_vm_uword_t             pc = vm->pc;

    see_vm_opcode_t opcode;
    int             type;
    see_vm_uword_t  arg;


    see_object_t *stack_entry     = vm->stack.entry;
    see_uintptr_t stack_size      = vm->stack.size;
    see_uintptr_t stack_allocated = vm->stack.allocated;

    
    while (1)
    {
        if (pc >= opcode_count) {
            exception->type = SEE_VM_EXCEPTION_TYPE_INVALID_PC;
            goto exit;
        }
        
        opcode = opcode_entry[pc];
        type   = SEE_VM_OPCODE_TYPE(opcode);
        arg    = SEE_VM_OPCODE_TYPE(opcode);

        switch (type)
        {
        case SEE_VM_OPCODE_TYPE_INLINE:
            switch (arg)
            {
            case SEE_VM_OPCODE_INLINE_NOP: break;
            case SEE_VM_OPCODE_INLINE_IADD:
                if (stack_size < 2)
                {
                    exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                    goto exit;
                }
                stack_entry[stack_size - 2] =
                    SEE_INTERNAL_INT_BOX(
                        SEE_INTERNAL_INT_UNBOX(stack_entry[stack_size - 2]) +
                        SEE_INTERNAL_INT_UNBOX(stack_entry[stack_size - 1]));
                -- stack_size;
                break;
                
            case SEE_VM_OPCODE_INLINE_ISUB:
                if (stack_size < 2)
                {
                    exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                    goto exit;
                }
                stack_entry[stack_size - 2] =
                    SEE_INTERNAL_INT_BOX(
                        SEE_INTERNAL_INT_UNBOX(stack_entry[stack_size - 2]) -
                        SEE_INTERNAL_INT_UNBOX(stack_entry[stack_size - 1]));
                -- stack_size;
                break;
                
            default:
                exception->type = SEE_VM_EXCEPTION_TYPE_UNSUPPORTED;
                goto exit;
            }
            ++ pc;
            break;
        
        case SEE_VM_OPCODE_TYPE_CONSTANT:
            if (arg >= prog->constant_count)
            {
                exception->type = SEE_VM_EXCEPTION_TYPE_OVERFLOW;
                goto exit;
            }
            
            if (VM_PUSH(stack_entry, stack_size, stack_allocated, prog->constant_pool[arg]))
            {
                exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                goto exit;
            }
            
            ++ pc;
            break;
        
        case SEE_VM_OPCODE_TYPE_VAR: break;
        case SEE_VM_OPCODE_TYPE_POP:
            if (stack_size < arg)
            {
                exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                goto exit;
            }

            stack_size -= arg;
            ++ pc;
            break;
            
        case SEE_VM_OPCODE_TYPE_CLOSURE: break;
        case SEE_VM_OPCODE_TYPE_APPLY: break;
        case SEE_VM_OPCODE_TYPE_NJUMP:
            if (stack_size > 0)
            {
                if (stack_entry[stack_size - 1] == SEE_OBJECT_NULL)
                    pc = arg;
                else ++ pc;
                -- stack_size;
            }
            else
            {
                exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                goto exit;
            }
            break;
        default:
            exception->type = SEE_VM_EXCEPTION_TYPE_UNSUPPORTED;
            goto exit;
            break;
        }
    }

  exit:

    VM_COMPACT(stack_entry, stack_size, stack_allocated);
    
    vm->stack.entry     = stack_entry;
    vm->stack.size      = stack_size;
    vm->stack.allocated = stack_allocated;
    
    vm->prog = prog;
    vm->pc   = pc;
}
