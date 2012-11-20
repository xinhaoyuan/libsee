#include <stdio.h>
#include <see/vm/vm.h>

#define VM_PUSH(entry, size, allocated) ({                      \
            typeof(entry) ne = entry;                           \
            if ((size) == (allocated))                          \
            {                                                   \
                typeof(size) na;                                \
                if (allocated)                                  \
                    na = allocated << 1;                        \
                else na = 16;                                   \
                ne = SEE_REALLOC(ne, na * sizeof(entry[0]));    \
                if (ne) {                                       \
                    allocated = na;                             \
                    entry = ne;                                 \
                }                                               \
            }                                                   \
            ne == NULL ? NULL :                                 \
                (entry = ne, &entry[size ++]);                  \
        })

#define VM_COMPACT(entry, size, allocated) ({                       \
            typeof(size)  na = allocated;                           \
            typeof(entry) ne = entry;                               \
            while (size < (na >> 1))                                \
                na >>= 1;                                           \
            ne = SEE_REALLOC(entry, na * sizeof(entry[0]));         \
            ne == NULL ? NULL : (allocated = na, entry = ne);       \
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

    see_vm_stack_frame_s *sf_stack_entry     = vm->sf_stack.entry;
    see_uintptr_t         sf_stack_size      = vm->sf_stack.size;
    see_uintptr_t         sf_stack_allocated = vm->sf_stack.allocated;
    
    see_vm_slot_s *op_stack_entry     = vm->op_stack.entry;
    see_uintptr_t  op_stack_size      = vm->op_stack.size;
    see_uintptr_t  op_stack_allocated = vm->op_stack.allocated;
    see_uintptr_t  op_stack_bound     = vm->op_stack_bound;

    while (1)
    {
        if (pc >= opcode_count) {
            exception->type = SEE_VM_EXCEPTION_TYPE_INVALID_PC;
            goto exit;
        }
        
        opcode = opcode_entry[pc];
        type   = SEE_VM_OPCODE_TYPE(opcode);
        arg    = SEE_VM_OPCODE_ARG(opcode);

        /* printf("%ld : opcode %016lx\n", pc, opcode); */
        /* see_uintptr_t i; */
        /* for (i = 0; i < op_stack_size; ++ i) */
        /*     printf("%016lx\n", op_stack_entry[i]._word); */

        switch (type)
        {
        case SEE_VM_OPCODE_TYPE_NO_ARG:
            switch (arg)
            {
            case SEE_VM_OPCODE_NA_NOP: ++ pc; break;
                
            case SEE_VM_OPCODE_NA_EQ:
                if (op_stack_size < op_stack_bound + 2)
                {
                    exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                    goto exit;
                }

                int eq = 0;
                see_vm_slot_t a = &op_stack_entry[op_stack_size - 2];
                see_vm_slot_t b = &op_stack_entry[op_stack_size - 1];
                
                if (a->type == b->type)
                {
                    switch (a->type)
                    {
                    case SEE_VM_SLOT_TYPE_OBJECT:
                        eq = a->_object == b->_object;
                        break;

                    case SEE_VM_SLOT_TYPE_WORD:
                        eq = a->_word == b->_word;
                        break;

                    default:
                        exception->type = SEE_VM_EXCEPTION_TYPE_UNSUPPORTED;
                        goto exit;
                        
                    }
                }

                a->type = SEE_VM_SLOT_TYPE_OBJECT;
                a->_object = eq ? SEE_OBJECT_TRUE : SEE_OBJECT_NULL;
                
                -- op_stack_size;
                ++ pc;
                break;

            case SEE_VM_OPCODE_NA_IADD:
                if (op_stack_size < op_stack_bound + 2)
                {
                    exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                    goto exit;
                }
                else
                {
                    op_stack_entry[op_stack_size - 2].type   = SEE_VM_SLOT_TYPE_WORD;
                    op_stack_entry[op_stack_size - 2]._word += op_stack_entry[op_stack_size - 1]._word;
                }
                -- op_stack_size;
                ++ pc;
                break;
                
            case SEE_VM_OPCODE_NA_ISUB:
                if (op_stack_size < op_stack_bound + 2)
                {
                    exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                    goto exit;
                }
                else
                {
                    op_stack_entry[op_stack_size - 2].type   = SEE_VM_SLOT_TYPE_WORD;
                    op_stack_entry[op_stack_size - 2]._word -= op_stack_entry[op_stack_size - 1]._word;
                }
                -- op_stack_size;
                ++ pc;
                break;

            case SEE_VM_OPCODE_NA_ENTER:
            {
                see_vm_stack_frame_t sf = VM_PUSH(sf_stack_entry, sf_stack_size, sf_stack_allocated);
                
                if (sf == NULL)
                {
                    exception->type = SEE_VM_EXCEPTION_TYPE_NO_MEM;
                    goto exit;
                }

                sf->envir = vm->envir;
                sf->prog  = prog;
                // Leave sf->pc empty to be filled by apply
                sf->op_stack_bound = op_stack_bound;

                op_stack_bound = op_stack_size;
                ++ pc;
                break;
            }

            case SEE_VM_OPCODE_NA_RETURN:
            {
                see_vm_stack_frame_t sf;
                if (sf_stack_size > 0 || op_stack_size <= op_stack_bound)
                {
                    /* Need a valid stack frame and a return value */
                    exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                    goto exit;
                }

                /* Pop the stack frame */
                sf = &sf_stack_entry[-- sf_stack_size];

                /* Move the return value */
                op_stack_entry[op_stack_bound] = op_stack_entry[op_stack_size - 1];

                op_stack_size  = op_stack_bound;
                op_stack_bound = sf->op_stack_bound;
                vm->envir      = sf->envir;
                prog           = sf->prog;
                pc             = sf->pc;

                /* We dont care about the error */
                VM_COMPACT(op_stack_entry, op_stack_size, op_stack_allocated);
                break;
            }
        
            case SEE_VM_OPCODE_NA_APPLY:
            {
                if (op_stack_size > op_stack_bound)
                {
                    if (op_stack_entry[op_stack_size - 1].type != SEE_VM_SLOT_TYPE_OBJECT)
                    {
                        exception->type = SEE_VM_EXCEPTION_TYPE_UNHANDLED_APPLY;
                        goto exit;
                    }

                    see_object_t      object = op_stack_entry[op_stack_size - 1]._object;
                    see_simple_type_t type   = SEE_OBJECT_SIMPLE_TYPE_GET(object);

                    switch (type)
                    {
                    case (see_simple_type_t)SEE_TYPE_VM_CLOSURE:
                    {
                        /* Update the return pc */
                        if (sf_stack_size > 0)
                        {
                            sf_stack_entry[sf_stack_size - 1].pc = pc + 1;
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
                see_vm_slot_t slot = VM_PUSH(op_stack_entry, op_stack_size, op_stack_allocated);
                if (slot)
                    *slot = prog->constant_slot[arg];
                else
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

        case SEE_VM_OPCODE_TYPE_DUP:
            ++ arg;
            if (arg == 0 || op_stack_size < op_stack_bound + arg)
            {
                exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                goto exit;
            }

            see_vm_slot_t slot = VM_PUSH(op_stack_entry, op_stack_size, op_stack_allocated);
            if (slot)
                *slot = op_stack_entry[op_stack_size - arg - 1];
            else 
            {
                exception->type = SEE_VM_EXCEPTION_TYPE_NO_MEM;
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

            see_vm_slot_t slot = VM_PUSH(op_stack_entry, op_stack_size, op_stack_allocated);
            if (slot)
                *slot = cur->slot[arg];
            else 
            {
                exception->type = SEE_VM_EXCEPTION_TYPE_NO_MEM;
                goto exit;
            }
            
            ++pc;
            break;
        }
        
        case SEE_VM_OPCODE_TYPE_POP:
            if (op_stack_size < op_stack_bound + arg)
            {
                exception->type = SEE_VM_EXCEPTION_TYPE_ARG_ERROR;
                goto exit;
            }

            op_stack_size -= arg;
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

            see_vm_slot_t slot = VM_PUSH(op_stack_entry, op_stack_size, op_stack_allocated);
            if (slot)
            {
                slot->type = SEE_VM_SLOT_TYPE_OBJECT;
                slot->_object = closure;
                see_object_unprotect(closure);
            }
            else
            {
                see_object_unprotect(closure);
                exception->type = SEE_VM_EXCEPTION_TYPE_NO_MEM;
                goto exit;
            }
            ++ pc;
            break;
        }
            
        case SEE_VM_OPCODE_TYPE_COND_JUMP:
            if (op_stack_size > op_stack_bound)
            {
                if (op_stack_entry[op_stack_size - 1].type == SEE_VM_SLOT_TYPE_OBJECT &&
                    op_stack_entry[op_stack_size - 1]._object == SEE_OBJECT_NULL)
                    pc = arg;
                else ++ pc;
                -- op_stack_size;
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

    VM_COMPACT(op_stack_entry, op_stack_size, op_stack_allocated);
    VM_COMPACT(sf_stack_entry, sf_stack_size, sf_stack_allocated);
    
    vm->op_stack.entry     = op_stack_entry;
    vm->op_stack.size      = op_stack_size;
    vm->op_stack.allocated = op_stack_allocated;
    vm->op_stack_bound     = op_stack_bound;

    vm->sf_stack.entry     = sf_stack_entry;
    vm->sf_stack.size      = sf_stack_size;
    vm->sf_stack.allocated = sf_stack_allocated;

    vm->prog = prog;
    vm->pc   = pc;
}
