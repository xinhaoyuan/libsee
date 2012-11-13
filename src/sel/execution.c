#include <see/sel/execution.h>
#include <stdarg.h>

typedef struct see_sel_environment_s *see_sel_environment_t;
typedef struct see_sel_label_s       *see_sel_label_t;

typedef struct see_sel_environment_s
{
    see_type_t type;
    
    see_uintptr_t length;
    see_slot_s   *slot_entry;
    see_object_t  parent;
} see_sel_environment_s;

typedef struct see_sel_label_s
{
    see_type_t type;
    
    see_object_t    envir;
    see_sel_label_f entry;
    see_uintptr_t   argc;
} see_sel_label_s;

static const char *label_type_name(see_type_t self, see_object_t object){ return "SEE_SEL_LABEL"; }
static void label_type_enumerate(see_type_t self, see_object_t object, void(*touch)(void *, see_object_t), void *priv)
{
    see_sel_label_t label = (see_sel_label_t)object;
    touch(priv, label->envir);
}
static void label_type_free(see_type_t self, see_object_t object) { }
static see_type_s label_type = {
    .name      = label_type_name,
    .enumerate = label_type_enumerate,
    .free      = label_type_free,
};

static const char *envir_type_name(see_type_t self, see_object_t object){ return "SEE_SEL_ENVIRONMENT"; }
static void envir_type_enumerate(see_type_t self, see_object_t object, void(*touch)(void *, see_object_t), void *priv)
{
    see_sel_environment_t envir = (see_sel_environment_t)object;
    see_uintptr_t i;
    for (i = 0; i < envir->length; ++ i)
        touch(priv, SEE_SLOT_GET(envir->slot_entry[i]));
    touch(priv, envir->parent);
}
static void envir_type_free(see_type_t self, see_object_t object) {
    see_sel_environment_t envir = (see_sel_environment_t)object;
    if (envir->length) SEE_FREE(envir->slot_entry);
}
static see_type_s envir_type = {
    .name      = envir_type_name,
    .enumerate = envir_type_enumerate,
    .free      = envir_type_free,
};

see_object_t
see_sel_label_new(see_sel_execution_t execution, see_sel_label_f entry, int argc)
{
    if (execution)
    {
        see_sel_label_t l = SEE_OBJECT_NEW(execution->heap, see_sel_label_s);
        if (l == NULL) return NULL;
        
        l->envir = execution->envir;
        l->entry = entry;
        l->argc  = argc;

        SEE_OBJECT_TYPE_INIT(l, &label_type);
        return l;
    }
    else
    {
        /* host label -- for builtin functions */
        see_sel_label_t l = SEE_OBJECT_NEW(NULL, see_sel_label_s);
        if (l == NULL) return NULL;
        
        l->envir = SEE_OBJECT_NULL;
        l->entry = entry;
        l->argc  = argc;

        SEE_OBJECT_TYPE_INIT(l, &label_type);
        return l;
    }
}

see_object_t
see_sel_local_ref(see_sel_execution_t execution, see_uintptr_t level, see_uintptr_t offset)
{
    see_object_t envir_level = execution->envir;
    while (SEE_OBJECT_TYPE(envir_level) == &envir_type &&
           level > 0)
    {
        envir_level = ((see_sel_environment_t)envir_level)->parent;
        -- level;
    }

    if (level == 0 && SEE_OBJECT_TYPE(envir_level) == &envir_type &&
        ((see_sel_environment_t)envir_level)->length > offset)
    {
        see_sel_environment_t envir = (see_sel_environment_t)envir_level;
        see_object_t result  = SEE_SLOT_GET(envir->slot_entry[offset]);
        see_object_protect(result);
        return result;
    }
    else
    {
        return SEE_OBJECT_NULL;
    }
}

#include <string.h>
#include <stdio.h>

void
see_sel_global_ref_and_goto(see_sel_execution_t execution, const char *name, see_object_t object)
{
    see_object_t result = SEE_OBJECT_NULL;
    /* XXX: add some builtin functions here */
    see_sel_goto(execution, object, result, NULL);
}

void
see_sel_goto(see_sel_execution_t execution, see_object_t object, ...)
{
    if (SEE_OBJECT_TYPE(object) != &label_type)
        goto release_and_return;

    va_list ap;
    see_object_t now;
    see_sel_label_t label = (see_sel_label_t)object;
    see_sel_environment_t envir;
    
    if (label->argc > 0)
    {
        envir = SEE_OBJECT_NEW(execution->heap, see_sel_environment_s);
        if (envir == NULL)
            goto release_and_return;
        
        envir->length = label->argc;
        envir->parent = label->envir;
    
        if ((envir->slot_entry =
             (see_slot_s *)SEE_MALLOC(sizeof(see_slot_s) * envir->length)) == NULL)
        {
            see_object_unprotect(envir);
            goto release_and_return;
        }
        
        see_uintptr_t i = 0;
        
        va_start(ap, object);
        while ((now = va_arg(ap, see_object_t)) != NULL)
        {
            if (i >= label->argc) break;
            SEE_SLOT_INIT(envir->slot_entry[i], now);
            ++ i;
        }
        va_end(ap);
        
        for (; i < label->argc; ++ i)
            SEE_SLOT_INIT(envir->slot_entry[i], SEE_OBJECT_NULL);

        SEE_OBJECT_TYPE_INIT(envir, &envir_type);
    }
    else
    {
        envir = label->envir;
        see_object_protect(envir);
    }

    see_object_unprotect(execution->envir);
    execution->envir = envir;
    execution->pc    = label->entry;

  release_and_return:

    va_start(ap, object);
    while ((now = va_arg(ap, see_object_t)) != NULL)
    {
        see_object_unprotect(now);
    }
    va_end(ap);
    see_object_unprotect(object);
}

int
see_sel_with(see_sel_execution_t execution, see_uintptr_t size)
{
    see_sel_environment_t envir = SEE_OBJECT_NEW(execution->heap, see_sel_environment_s);
    if (envir == NULL)
        return -1;
    
    envir->length = size;
    envir->parent = execution->envir;

    if ((envir->slot_entry = (see_slot_s *)SEE_MALLOC(sizeof(see_slot_s) * envir->length)) == NULL)
    {
        see_object_unprotect(envir);
        return -1;
    }

    for (size = 0; size < envir->length; ++ size)
        SEE_SLOT_INIT(envir->slot_entry[size], SEE_OBJECT_NULL);

    SEE_OBJECT_TYPE_INIT(envir, &envir_type);
    see_object_unprotect(execution->envir);
    execution->envir = envir;

    return 0;
}

void
see_sel_set_and_goto(see_sel_execution_t execution, see_uintptr_t level, see_uintptr_t offset, see_object_t value, see_object_t object)
{
    see_object_t envir_level = execution->envir;
    while (SEE_OBJECT_TYPE(envir_level) == &envir_type && level > 0)
    {
        envir_level = ((see_sel_environment_t)envir_level)->parent;
        -- level;
    }

    if (level == 0 && SEE_OBJECT_TYPE(envir_level) == &envir_type &&
        ((see_sel_environment_t)envir_level)->length > offset)
    {
        see_sel_environment_t envir = ((see_sel_environment_t)envir_level);
        see_object_t old  = SEE_SLOT_GET(envir->slot_entry[offset]);
        see_object_protect(old);
        SEE_SLOT_SET(envir->slot_entry[offset], value);
        see_object_unprotect(value);
        see_sel_goto(execution, object, old, NULL);
    }
    else
    {
        see_object_unprotect(value);
        see_sel_goto(execution, object, SEE_OBJECT_NULL, NULL);
    }
}

void
see_sel_global_set_and_goto(see_sel_execution_t execution, const char *name, see_object_t value, see_object_t object)
{
    /* No global variables */
    see_object_unprotect(value);
    see_sel_goto(execution, object, SEE_OBJECT_NULL, NULL);
}

see_sel_execution_t
see_sel_execution_new(void)
{
    see_sel_execution_t ex = (see_sel_execution_t)SEE_MALLOC(sizeof(see_sel_execution_s));
    if (ex == NULL) return NULL;

    if ((ex->heap = see_heap_new()) == NULL)
    {
        SEE_FREE(ex);
        return NULL;
    }

    ex->envir = SEE_OBJECT_NULL;
    ex->pc = NULL;
    
    return ex;
}

void
see_sel_execution_free(see_sel_execution_t execution)
{
    see_heap_free(execution->heap);
    SEE_FREE(execution);
}


static void
see_sel_step(see_sel_execution_t execution)
{
    see_sel_label_f now = execution->pc;
    if (now == NULL)
        return;
    execution->pc = NULL;
    now(execution);
}

void
see_sel_run(see_sel_execution_t execution)
{
    while (execution->pc)
        see_sel_step(execution);
}

void
see_sel_execution_sys_init(void)
{ }
