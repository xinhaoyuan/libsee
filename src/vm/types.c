#include <see/vm/types.h>

static const char *string_symbol_type_name(see_type_t self, see_object_t object) { return "VM_STRING_SYMBOL";}
static void string_symbol_type_enumerate(see_type_t self, see_object_t object, void(*touch)(void *, see_object_t), void *priv) { }
static void string_symbol_type_free(see_type_t self, see_object_t object)
{
    see_vm_string_symbol_t s = object;
    if (s->string) SEE_FREE(s->string);
}
static see_type_s string_symbol_type = {
    .name      = string_symbol_type_name,
    .enumerate = string_symbol_type_enumerate,
    .free      = string_symbol_type_free,
};


static const char *pair_type_name(see_type_t self, see_object_t object) { return "VM_PAIR";}
static void pair_type_enumerate(see_type_t self, see_object_t object, void(*touch)(void *, see_object_t), void *priv)
{
    see_vm_pair_t pair = object;
    if (pair->car.type == SEE_VM_SLOT_TYPE_OBJECT) touch(priv, pair->car._object);
    if (pair->cdr.type == SEE_VM_SLOT_TYPE_OBJECT) touch(priv, pair->cdr._object);
}
static void pair_type_free(see_type_t self, see_object_t object) {}
static see_type_s pair_type = {
    .name      = pair_type_name,
    .enumerate = pair_type_enumerate,
    .free      = pair_type_free,
};

static const char *vector_type_name(see_type_t self, see_object_t object) { return "VM_VECTOR";}
static void vector_type_enumerate(see_type_t self, see_object_t object, void(*touch)(void *, see_object_t), void *priv)
{
    see_vm_uword_t i;
    see_vm_vector_t v = object;
    for (i = 0; i < v->length; ++ i)
    {
        if (v->entry[i].type == SEE_VM_SLOT_TYPE_OBJECT)
            touch(priv, v->entry[i]._object);
    }
}
static void vector_type_free(see_type_t self, see_object_t object)
{
    see_vm_vector_t v = object;
    if (v->length) SEE_FREE(v->entry);
}
static see_type_s vector_type = {
    .name      = vector_type_name,
    .enumerate = vector_type_enumerate,
    .free      = vector_type_free,
};

static const char *closure_type_name(see_type_t self, see_object_t object) { return "VM_CLOSURE";}
static void closure_type_enumerate(see_type_t self, see_object_t object, void(*touch)(void *, see_object_t), void *priv)
{
    see_vm_closure_t c = (see_vm_closure_t)object;
    if (c->envir != NULL) touch(priv, c->envir);
    touch(priv, c->prog);
}
static void closure_type_free(see_type_t self, see_object_t object) { }
static see_type_s closure_type = {
    .name      = closure_type_name,
    .enumerate = closure_type_enumerate,
    .free      = closure_type_free,
};

static const char *prog_type_name(see_type_t self, see_object_t object) { return "VM_PROG";}
static void prog_type_enumerate(see_type_t self, see_object_t object, void(*touch)(void *, see_object_t), void *priv)
{
    see_vm_uword_t i;
    see_vm_prog_t p = object;
    for (i = 0; i < p->constant_count; ++ i)
    {
        if (p->constant_slot[i].type == SEE_VM_SLOT_TYPE_OBJECT)
            touch(priv, p->constant_slot[i]._object);
    }
}
static void prog_type_free(see_type_t self, see_object_t object)
{
    see_vm_prog_t p = object;
    if (p->constant_count > 0) SEE_FREE(p->constant_slot);
    if (p->opcode_count > 0)   SEE_FREE(p->opcode_entry);
}
static see_type_s prog_type = {
    .name      = prog_type_name,
    .enumerate = prog_type_enumerate,
    .free      = prog_type_free,
};

static const char *envir_type_name(see_type_t self, see_object_t object) { return "VM_ENVIR";}
static void envir_type_enumerate(see_type_t self, see_object_t object, void(*touch)(void *, see_object_t), void *priv)
{
    see_vm_uword_t i;
    see_vm_envir_t e = object;
    touch(priv, e->parent);
    for (i = 0; i < e->size; ++ i)
    {
        if (e->slot[i].type == SEE_VM_SLOT_TYPE_OBJECT)
            touch(priv, e->slot[i]._object);
    }
}
static void envir_type_free(see_type_t self, see_object_t object)
{
    see_vm_envir_t e = object;
    SEE_FREE(e->slot);
}
static see_type_s envir_type = {
    .name      = envir_type_name,
    .enumerate = envir_type_enumerate,
    .free      = envir_type_free,
};

static const char*
vm_type_name(see_type_t self, see_object_t object)
{ return "SEE_VM"; }
static void
vm_type_enumerate(see_type_t self, see_object_t object, void(*touch)(void *priv, see_object_t data), void *priv)
{
    see_uintptr_t i;
    see_vm_t vm = object;

    touch(priv, vm->envir);
    touch(priv, vm->prog);
    touch(priv, vm->dump);
    
    for (i = 0; i < vm->stack.size; ++ i)
    {
        if (vm->stack.entry[i].type == SEE_VM_SLOT_TYPE_OBJECT &&
            vm->stack.entry[i]._object != NULL)
            touch(priv, vm->stack.entry[i]._object);
    }
}
static void
vm_type_free(see_type_t self, see_object_t object)
{
    see_vm_t vm = object;
    SEE_FREE(vm->stack.entry);
}
static see_type_s vm_type = {
    .name      = vm_type_name,
    .enumerate = vm_type_enumerate,
    .free      = vm_type_free,
};

void
see_vm_types_sys_init(void)
{
    see_type_alias_register(SEE_TYPE_VM_STRING_SYMBOL, &string_symbol_type);
    see_type_alias_register(SEE_TYPE_VM_PAIR,          &pair_type);
    see_type_alias_register(SEE_TYPE_VM_VECTOR,        &vector_type);
    see_type_alias_register(SEE_TYPE_VM_CLOSURE,       &closure_type);
    see_type_alias_register(SEE_TYPE_VM_PROG,          &prog_type);
    see_type_alias_register(SEE_TYPE_VM_ENVIR,         &envir_type);
    see_type_alias_register(SEE_TYPE_VM,               &vm_type);
}

see_vm_closure_t
see_vm_closure_new(see_heap_t heap, see_vm_envir_t envir, see_vm_prog_t prog, see_vm_uword_t entry)
{
    see_vm_closure_t closure = SEE_OBJECT_NEW(heap, see_vm_closure_s);
    if (closure == NULL) return NULL;

    closure->envir = envir;
    closure->prog  = prog;
    closure->entry = entry;

    SEE_OBJECT_TYPE_INIT(closure, SEE_TYPE_VM_CLOSURE);

    return closure;
}

/* XXX now using the host byte order */

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

    if ((prog->constant_slot =
         (see_vm_slot_s *)SEE_MALLOC(constant_count * sizeof(see_vm_slot_s)))
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
        SEE_FREE(prog->constant_slot);
        if (heap == NULL)
            see_object_host_free(prog);
        else see_object_unprotect(prog);
        return NULL;
    }

    prog->constant_count = constant_count;
    prog->opcode_count   = opcode_count;

    see_vm_uword_t i;
    
    /* XXX read object constant */
    for (i = 0; i < constant_count; ++ i)
    {
        see_vm_word_t word;
        if (READ_VAR(word, in, data))
        {
            prog->constant_slot[i].type  = SEE_VM_SLOT_TYPE_WORD;
            prog->constant_slot[i]._word = word;
        }
        else
        {
            SEE_FREE(prog->constant_slot);
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
            SEE_FREE(prog->constant_slot);
            SEE_FREE(prog->opcode_entry);
            if (heap == NULL)
                see_object_host_free(prog);
            else see_object_unprotect(prog);
            return NULL;
        }
    }

    SEE_OBJECT_TYPE_INIT(prog, SEE_TYPE_VM_PROG);

    return prog;
}

see_vm_t
see_vm_new(see_heap_t heap, see_vm_prog_t prog)
{
    see_vm_t vm = SEE_OBJECT_NEW(heap, see_vm_s);
    if (vm == NULL) return NULL;

    vm->heap = heap;

    vm->stack.allocated = 0;
    vm->stack.size      = 0;
    vm->stack.bound     = 0;
    vm->stack.entry     = NULL;

    vm->envir = NULL;
    vm->prog  = prog;
    vm->pc    = 0;
    vm->dump  = NULL;

    SEE_OBJECT_TYPE_INIT(vm, SEE_TYPE_VM);

    return vm;
}
