#ifndef __SEE_VM_PROG_H__
#define __SEE_VM_PROG_H__

#include <see/config.h>
#include <see/vm/types.h>
#include <see/data/object.h>
#include <see/io/char_stream.h>
#include <see/vm/opcode.h>

typedef struct see_vm_prog_s *see_vm_prog_t;
typedef struct see_vm_prog_s
{
    see_vm_uword_t   constant_count;
    see_object_t    *constant_pool;
    see_vm_uword_t   opcode_count;
    see_vm_opcode_t *opcode_entry;
} see_vm_prog_s;

/* Create a vm prog by reading stream in human-readable format */
see_vm_prog_t see_vm_prog_parse_text(see_heap_t heap, see_io_char_stream_in_f in, void *data);
/* And in compact binary format */
see_vm_prog_t see_vm_prog_parse_bin(see_heap_t heap, see_io_char_stream_in_f in, void *data);

#endif
