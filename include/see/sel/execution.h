#ifndef __SEE_SEL_EXECUTION_H__
#define __SEE_SEL_EXECUTION_H__

#include "../data/object.h"

typedef struct see_sel_execution_s   *see_sel_execution_t;
typedef void(*see_sel_label_f)(see_sel_execution_t);

typedef struct see_sel_execution_s
{
    see_heap_t        heap;
    see_object_t      envir;
    see_sel_label_f   pc;
} see_sel_execution_s;

#define SEE_SEL_MAKE_NUMBER(execution, num)           (SEE_INT_BOX(num))
#define SEE_SEL_MAKE_INTEGER(execution, num)          (SEE_INT_BOX(num))
#define SEE_SEL_MAKE_STIRNG_SYMBOL(execution, string) (see_string_symbol_new(execution, string))
#define SEE_SEL_IS_TRUE(object)                       (object == SEE_OBJECT_NULL)

void see_sel_execution_sys_init(void);

/* execution == NULL creates host label */
see_object_t        see_sel_label_new(see_sel_execution_t execution, see_sel_label_f entry, int argc);
see_object_t        see_sel_local_ref(see_sel_execution_t execution, see_uintptr_t level, see_uintptr_t offset);
void                see_sel_global_ref_and_goto(see_sel_execution_t execution, const char *name, see_object_t object);
void                see_sel_goto(see_sel_execution_t execution, see_object_t label, ...);
void                see_sel_set_and_goto(see_sel_execution_t execution, see_uintptr_t level, see_uintptr_t offset, see_object_t value, see_object_t object);
void                see_sel_global_set_and_goto(see_sel_execution_t execution, const char *name, see_object_t value, see_object_t object);
int                 see_sel_with(see_sel_execution_t execution, see_uintptr_t size);
see_sel_execution_t see_sel_execution_new(void);
void                see_sel_execution_free(see_sel_execution_t execution);
void                see_sel_run(see_sel_execution_t execution);

#endif
