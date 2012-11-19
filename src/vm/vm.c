#include <stdio.h>
#include <see/vm/vm.h>

#define VM_PUSH_SLOT(entry, size, allocated, slot) ({               \
            see_vm_slot_s *ne = entry;                              \
            if (size == allocated)                                  \
            {                                                       \
                see_uintptr_t na;                                   \
                if (allocated)                                      \
                    na = allocated << 1;                            \
                else na = 64;                                       \
                ne = SEE_REALLOC(ne, na * sizeof(see_vm_slot_s));   \
                allocated = na;                                     \
                entry = ne;                                         \
            }                                                       \
            ne == NULL ? -1 :                                       \
                (entry = ne,                                        \
                 entry[size] = (slot),                              \
                 ++ size, 0);                                       \
        })

#define VM_PUSH(entry, size, allocated, _type, field, data)  ({     \
            see_vm_slot_s *ne = entry;                              \
            if (size == allocated)                                  \
            {                                                       \
                see_uintptr_t na;                                   \
                if (allocated)                                      \
                    na = allocated << 1;                            \
                else na = 64;                                       \
                ne = SEE_REALLOC(ne, na * sizeof(see_vm_slot_s));   \
                allocated = na;                                     \
                entry = ne;                                         \
            }                                                       \
            ne == NULL ? -1 :                                       \
                (entry = ne,                                        \
                 entry[size].type     = (_type),                    \
                 entry[size]._##field = (data),                     \
                 ++ size,  0);                                      \
        })

#define VM_PUSH_OBJECT(entry, size, allocated, data)                    \
    VM_PUSH(entry, size, allocated, SEE_VM_SLOT_TYPE_OBJECT, object, data)

#define VM_PUSH_WORD(entry, size, allocated, data)                      \
    VM_PUSH(entry, size, allocated, SEE_VM_SLOT_TYPE_WORD, word, data)

#define VM_PUSH_UWORD(entry, size, allocated, data)                     \
    VM_PUSH(entry, size, allocated, SEE_VM_SLOT_TYPE_UWORD, uword, data)

#define VM_PUSH_PTR(entry, size, allocated, data)                       \
    VM_PUSH(entry, size, allocated, SEE_VM_SLOT_TYPE_PTR, ptr, data)


#define VM_COMPACT(entry, size, allocated) ({                       \
            see_uintptr_t  na = allocated;                          \
            see_vm_slot_s *ne = entry;                              \
            while (size < (na >> 1))                                \
                na >>= 1;                                           \
            allocated = na;                                         \
            ne = SEE_REALLOC(entry, na * sizeof(see_vm_slot_s));    \
            ne == NULL ? -1 : (entry = ne, 0);                      \
        })

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

    see_vm_slot_s *stack_entry     = vm->stack.entry;
    see_uintptr_t  stack_size      = vm->stack.size;
    see_uintptr_t  stack_bound     = vm->stack.bound;
    see_uintptr_t  stack_allocated = vm->stack.allocated;
    
    while (1)
    {
        if (pc >= opcode_count) {
            exception->type = SEE_VM_EXCEPTION_TYPE_INVALID_PC;
            goto exit;
        }
        
        opcode = opcode_entry[pc];
        type   = SEE_VM_OPCODE_TYPE(opcode);
        arg    = SEE_VM_OPCODE_ARG(opcode);

        printf("opcode %016lx\n", opcode);

        switch (type)
        {
        case SEE_VM_OPCODE_TYPE_NO_ARG:
            switch (arg)
            {
            case SEE_VM_OPCODE_NA_NOP: ++ pc; break;
            case SEE_VM_OPCODE_NA_IADD:
                if (stack_size < stack_bound + 2)
                {
                    exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                    goto exit;
                }
                else
                {
                    stack_entry[stack_size - 2].type   = SEE_VM_SLOT_TYPE_WORD;
                    stack_entry[stack_size - 2]._word += stack_entry[stack_size - 1]._word;
                }
                -- stack_size;
                ++ pc;
                break;
                
            case SEE_VM_OPCODE_NA_ISUB:
                if (stack_size < stack_bound + 2)
                {
                    exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                    goto exit;
                }
                else
                {
                    stack_entry[stack_size - 2].type   = SEE_VM_SLOT_TYPE_WORD;
                    stack_entry[stack_size - 2]._word -= stack_entry[stack_size - 1]._word;
                }
                -- stack_size;
                ++ pc;
                break;

            case SEE_VM_OPCODE_NA_ENTER:
            {
                see_uintptr_t old_stack_size = stack_size;
                if (VM_PUSH_OBJECT(stack_entry, stack_size, stack_allocated, vm->envir))
                {
                    stack_size = old_stack_size;
                    exception->type = SEE_VM_EXCEPTION_TYPE_NO_MEM;
                    goto exit;
                }
                else if (VM_PUSH_OBJECT(stack_entry, stack_size, stack_allocated, prog))
                {
                    stack_size = old_stack_size;
                    exception->type = SEE_VM_EXCEPTION_TYPE_NO_MEM;
                    goto exit;
                }
                /* Save a temporary pc here, will be filled by apply */
                else if (VM_PUSH_UWORD(stack_entry, stack_size, stack_allocated, pc + 1))
                {
                    stack_size = old_stack_size;
                    exception->type = SEE_VM_EXCEPTION_TYPE_NO_MEM;
                    goto exit;
                }
                else if (VM_PUSH_PTR(stack_entry, stack_size, stack_allocated, (void *)stack_bound))
                {
                    stack_size = old_stack_size;
                    exception->type = SEE_VM_EXCEPTION_TYPE_NO_MEM;
                    goto exit;
                }

                stack_bound = stack_size;
                ++ pc;
                break;
            }

            case SEE_VM_OPCODE_NA_RETURN:
            {
                see_uintptr_t  n_stack_bound;
                see_vm_uword_t n_pc;
                see_object_t   n_prog;
                see_object_t   n_envir;

                if (stack_bound < 4 || stack_size <= stack_bound)
                {
                    exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                    goto exit;
                }

#if SEE_VM_STACK_CHECK_STRICT
                if (stack_entry[stack_bound - 1].type != SEE_VM_SLOT_TYPE_PTR)
                {
                    exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                    goto exit;
                }

                if (stack_entry[stack_bound - 2].type != SEE_VM_SLOT_TYPE_UWORD)
                {
                    exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                    goto exit;
                }

                if (stack_entry[stack_bound - 3].type != SEE_VM_SLOT_TYPE_OBJECT)
                {
                    exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                    goto exit;
                }

                if (stack_entry[stack_bound - 4].type != SEE_VM_SLOT_TYPE_OBJECT)
                {
                    exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                    goto exit;
                }
#endif

                n_stack_bound = (see_uintptr_t)stack_entry[stack_bound - 1]._ptr;
                n_pc          = stack_entry[stack_bound - 2]._uword;
                n_prog        = stack_entry[stack_bound - 3]._object;
                n_envir       = stack_entry[stack_bound - 4]._object;

#if SEE_VM_STACK_CHECK_STRICT
                if (SEE_OBJECT_SIMPLE_TYPE_GET(n_prog)  != (see_simple_type_t)SEE_TYPE_VM_PROG ||
                    (n_envir != NULL &&
                     SEE_OBJECT_SIMPLE_TYPE_GET(n_envir) != (see_simple_type_t)SEE_TYPE_VM_ENVIR))
                {
                    exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                    goto exit;
                }
#endif

                /* Move the return value */
                stack_entry[stack_bound - 4] = stack_entry[stack_size - 1];
                
                stack_size  = stack_bound;
                stack_bound = n_stack_bound;
                vm->envir   = n_envir;
                prog        = n_prog;
                pc          = n_pc;

                VM_COMPACT(stack_entry, stack_size, stack_allocated);
                break;
            }
        
            case SEE_VM_OPCODE_NA_APPLY:
            {
                if (stack_size > stack_bound)
                {
                    if (stack_entry[stack_size - 1].type != SEE_VM_SLOT_TYPE_OBJECT)
                    {
                        exception->type = SEE_VM_EXCEPTION_TYPE_UNHANDLED_APPLY;
                        goto exit;
                    }

                    see_object_t      object = stack_entry[stack_size - 1]._object;
                    see_simple_type_t type   = SEE_OBJECT_SIMPLE_TYPE_GET(object);

                    switch (type)
                    {
                    case (see_simple_type_t)SEE_TYPE_VM_CLOSURE:
                    {
                        /* Update the return pc */
                        if (stack_bound >= 2)
                        {
                            stack_entry[stack_bound - 2].type  = SEE_VM_SLOT_TYPE_WORD;
                            stack_entry[stack_bound - 2]._word = pc + 1;
                        }
                        see_vm_closure_t closure = object;
                        vm->envir = closure->envir;
                        prog      = closure->prog;
                        pc        = closure->entry;
                        break;
                    }

                    default:
                        exception->type = SEE_VM_EXCEPTION_TYPE_UNHANDLED_APPLY;
                        goto exit;
                    }

                }
                else
                {
                    exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                    goto exit;
                }
                break;
            }

            default:
                exception->type = SEE_VM_EXCEPTION_TYPE_UNSUPPORTED;
                goto exit;
            }
            break;
        
        case SEE_VM_OPCODE_TYPE_CONSTANT:
            if (arg < prog->constant_count)
            {
                if (VM_PUSH_SLOT(stack_entry, stack_size, stack_allocated, prog->constant_slot[arg]))
                {
                    exception->type = SEE_VM_EXCEPTION_TYPE_NO_MEM;
                    goto exit;
                }
            }
            else
            {
                exception->type = SEE_VM_EXCEPTION_TYPE_OVERFLOW;
                goto exit;
            }
            ++ pc;
            break;
        
        case SEE_VM_OPCODE_TYPE_VAR:
        {
            see_vm_envir_t cur = vm->envir;
            while (cur != NULL && arg >= cur->size)
            {
                arg -= cur->size;
                cur  = cur->parent;
            }
            
            if (cur == NULL)
            {
                exception->type = SEE_VM_EXCEPTION_TYPE_OVERFLOW;
                goto exit;
            }
            
            if (VM_PUSH_SLOT(stack_entry, stack_size, stack_allocated, cur->slot[arg]))
            {
                exception->type = SEE_VM_EXCEPTION_TYPE_NO_MEM;
                goto exit;
            }
            break;
        }
        
        case SEE_VM_OPCODE_TYPE_POP:
            if (stack_size < stack_bound + arg)
            {
                exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                goto exit;
            }

            stack_size -= arg;
            ++ pc;
            break;
            
        case SEE_VM_OPCODE_TYPE_CLOSURE:
        {
            see_vm_closure_t closure = see_vm_closure_new(vm->heap, vm->envir, prog, arg);
            if (closure == NULL)
            {
                exception->type = SEE_VM_EXCEPTION_TYPE_NO_MEM;
                goto exit;
            }
            else if (VM_PUSH_OBJECT(stack_entry, stack_size, stack_allocated, closure))
            {
                see_object_unprotect(closure);
                exception->type = SEE_VM_EXCEPTION_TYPE_NO_MEM;
                goto exit;
            }
            ++ pc;
            break;
        }
            
        case SEE_VM_OPCODE_TYPE_COND_JUMP:
            if (stack_size > stack_bound)
            {
                if (stack_entry[stack_size - 1].type == SEE_VM_SLOT_TYPE_OBJECT &&
                    stack_entry[stack_size - 1]._object == SEE_OBJECT_NULL)
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

        case SEE_VM_OPCODE_TYPE_JUMP:
            pc = arg;
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
    vm->stack.bound     = stack_bound;
    vm->stack.allocated = stack_allocated;

    vm->prog = prog;
    vm->pc   = pc;
}
