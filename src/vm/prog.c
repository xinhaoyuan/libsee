#include <see/vm/prog.h>

static int
read(char *buf, see_uintptr_t size, see_io_char_stream_in_f in, void *data)
{
    int i;
    for (i = 0; i < size; ++i)
    {
        int c = in(data, 1);
        if (c < 0) break;
        buf[i] = c;
    }
    return i;
}

#define READ_VAR(lv, in, data) (sizeof(lv) == read((char *)&(lv), sizeof(lv), (in), (data)))
    
see_vm_prog_t
see_vm_prog_parse_bin(see_heap_t heap, see_io_char_stream_in_f in, void *data)
{
    see_vm_uword_t constant_count;
    see_vm_uword_t opcode_count;
    if (!READ_VAR(constant_count, in, data)) return NULL;
    if (!READ_VAR(opcode_count, in, data)) return NULL;

    see_vm_prog_t prog = SEE_OBJECT_NEW(heap, see_vm_prog_s);
    if (prog == NULL) return NULL;

    if ((prog->constant_pool =
         (see_object_t *)SEE_MALLOC(constant_count * sizeof(see_object_t)))
        == NULL)
    {
        if (heap == NULL)
            see_object_host_free(prog);
        else see_object_unprotect(prog);
        return NULL;
    }
    
    if ((prog->opcode_entry =
         (see_vm_opcode_t *)SEE_MALLOC(opcode_count * sizeof(see_vm_opcode_t)))
        == NULL)
    {
        SEE_FREE(prog->constant_pool);
        if (heap == NULL)
            see_object_host_free(prog);
        else see_object_unprotect(prog);
        return NULL;
    }

    prog->constant_count = constant_count;
    prog->opcode_count   = opcode_count;

    see_vm_uword_t i;

    for (i = 0; i < constant_count; ++ i)
    {
        int failed = 0;
        see_vm_word_t num;
        /* read the constant */
        /* XXX: currently only internal integers */
        if (READ_VAR(num, in, data))
        {
            prog->constant_pool[i] = SEE_INTERNAL_INT_BOX(num);
        }
        else failed = 1;
        
        if (failed)
        {
            SEE_FREE(prog->constant_pool);
            SEE_FREE(prog->opcode_entry);
            if (heap == NULL)
                see_object_host_free(prog);
            else see_object_unprotect(prog);
            return NULL;
        }
    }

    for (i = 0; i < opcode_count; ++ i)
    {
        if (!READ_VAR(prog->opcode_entry[i], in, data))
        {
            SEE_FREE(prog->constant_pool);
            SEE_FREE(prog->opcode_entry);
            if (heap == NULL)
                see_object_host_free(prog);
            else see_object_unprotect(prog);
            return NULL;
        }
    }

    return prog;
}
