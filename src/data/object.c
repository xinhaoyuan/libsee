#include <see/data/object.h>

see_type_t see_types[SEE_TYPE_TOTAL];

#define HAS_GC_HEADER(object) (object && ((see_uintptr_t)(object) & SEE_TYPE_MASK) == 0)
#define TO_GC_HEADER(object)  ((see_gc_header_t)(object) - 1)
#define TO_OBJECT(gc)         ((see_object_t)((see_gc_header_t)(gc) + 1))

#define GC_MARK_UNTOUCH    0
#define GC_MARK_TOUCHED    1
#define GC_MARK_HOST       0xff

#define GC_THRESHOLD_INIT    4192
#define GC_QUEUE_LENGTH_INIT 256

see_type_t
see_object_type_get(see_object_t object)
{
    see_type_t suf = (see_type_t)((see_uintptr_t)object & SEE_TYPE_MASK);
    if (suf == NULL && object)
    {
        suf = (see_type_t)(see_uintptr_t)(TO_GC_HEADER(object)->type);
        if (suf == SEE_TYPE_EXTERNAL)
            /* For external objects, the type interface is stored as
             * the first member */
            return *(see_type_t *)object;
        return suf;
    }
    else return ((see_uintptr_t)suf & 1) ? (see_type_t)1 : suf;
}

see_simple_type_t
see_object_simple_type_get(see_object_t object)
{
    unsigned int suf = ((see_uintptr_t)object & SEE_TYPE_MASK);
    if (suf == 0 && object)
        return TO_GC_HEADER(object)->type;
    else return (suf & 1) ? 1 : suf;
}

void
see_object_type_init(see_object_t object, see_type_t type)
{
    if ((see_uintptr_t)type < SEE_TYPE_INTERNAL_TOTAL)
        type = SEE_TYPE_DUMMY;

    see_gc_header_t header = TO_GC_HEADER(object);
    if ((see_uintptr_t)type < SEE_TYPE_TOTAL)
    {
        header->type = (unsigned char)(see_uintptr_t)type;
    }
    else
    {
        header->type = (unsigned char)(see_uintptr_t)SEE_TYPE_EXTERNAL;
        *(see_type_t *)object = type;
    }
}

static int do_gc(see_heap_t heap);

see_object_t 
see_object_new_by_size(see_heap_t heap, see_uintptr_t size)
{
    if (heap && heap->object_count >= heap->gc_threshold)
    {
        if (do_gc(heap) != 0) return NULL;
    }

    see_gc_header_t gc = (see_gc_header_t)SEE_MALLOC(sizeof(see_gc_header_s) + size);
    if (gc == NULL) return NULL;

    gc->type          = (unsigned char)(see_uintptr_t)SEE_TYPE_DUMMY;
    gc->gc_mark       = heap ? GC_MARK_UNTOUCH : GC_MARK_HOST;
    gc->protect_level = 1;      /* objects are initially protected as returning values */

    if (heap)
    {
        heap->tracker[heap->object_count] = TO_OBJECT(gc);
        ++ heap->object_count;
    }
    
    return TO_OBJECT(gc);
}

void
see_object_host_free(see_object_t object)
{
    if (HAS_GC_HEADER(object) && TO_GC_HEADER(object)->gc_mark == GC_MARK_HOST)
    {
        see_gc_header_t gc = TO_GC_HEADER(object);
        see_types[gc->type]->free(see_types[gc->type], object);
        SEE_FREE(gc);
    }
}

int
see_object_protect(see_object_t object)
{
    if (HAS_GC_HEADER(object) && TO_GC_HEADER(object)->protect_level < SEE_PROTECT_LEVEL_MAX)
    {
        ++ TO_GC_HEADER(object)->protect_level;
        return 0;
    }
    else return -1;
}

int
see_object_unprotect(see_object_t object)
{
    if (HAS_GC_HEADER(object) && TO_GC_HEADER(object)->protect_level > 0)
    {
        -- TO_GC_HEADER(object)->protect_level;
        return 0;
    }
    else return -1;
}

typedef struct exqueue_s
{
    unsigned int alloc;
    unsigned int head;
    unsigned int tail;

    see_object_t *queue;
} exqueue_s;

static void
exqueue_enqueue(exqueue_s *q, see_object_t object)
{
    if (q->queue && HAS_GC_HEADER(object) && TO_GC_HEADER(object)->gc_mark == GC_MARK_UNTOUCH)
    {
        TO_GC_HEADER(object)->gc_mark = GC_MARK_TOUCHED;
        
        q->queue[q->head ++] = object;
        q->head %= q->alloc;
        if (q->head == q->tail)
        {
            void *new = SEE_REALLOC(q->queue, sizeof(see_object_t) * (q->alloc << 1));
            if (new == NULL)
            {
                free(q->queue);
                q->queue = NULL;
            }
            else q->queue = (see_object_t *)new;
            
            SEE_MEMCPY(q->queue + q->alloc, q->queue, sizeof(see_object_t) * q->head);
            q->head += q->alloc;
            q->alloc <<= 1;
        }
    }
}

static int
do_gc(see_heap_t heap)
{
    exqueue_s q;
    q.alloc = GC_QUEUE_LENGTH_INIT;
    q.head  = 0;
    q.tail  = 0;
    q.queue = (see_object_t *)SEE_MALLOC(sizeof(see_object_t) * q.alloc);

    see_uintptr_t i;
    see_gc_header_t gc;
    
    for (i = 0; i < heap->object_count; ++ i)
    {
        gc = TO_GC_HEADER(heap->tracker[i]);
        if (gc->protect_level)
        {
            exqueue_enqueue(&q, heap->tracker[i]);
            gc->gc_mark = GC_MARK_TOUCHED;
        }
        else
        {
            gc->gc_mark = GC_MARK_UNTOUCH;
        }
    }

    while (q.head != q.tail)
    {
        see_object_t now = q.queue[q.tail ++];
        gc = TO_GC_HEADER(now);
        q.tail %= q.alloc;

        see_types[gc->type]->enumerate(see_types[gc->type], now, (void(*)(void *,see_object_t))exqueue_enqueue, &q);
    }

    if (q.queue == NULL) return -1;
    SEE_FREE(q.queue);

    see_uintptr_t count_new = 0;
    for (i = 0; i < heap->object_count; ++ i)
    {
        gc = TO_GC_HEADER(heap->tracker[i]);
        if (gc->gc_mark == GC_MARK_TOUCHED)
            heap->tracker[count_new ++] = heap->tracker[i];
        else if (gc->gc_mark == GC_MARK_UNTOUCH)
        {
            see_types[gc->type]->free(see_types[gc->type], heap->tracker[i]);
            SEE_FREE(gc);
        }
    }

    see_uintptr_t threshold_new;
    
    /* Naive calcuation, should use binary instructions if possible */
    /* Should not overflow because the object count should be much
     * smaller than MAX_INTPTR */
    threshold_new = 1;
    while (threshold_new < count_new * 2)
        threshold_new = threshold_new << 1;
    if (threshold_new < GC_THRESHOLD_INIT)
        threshold_new = GC_THRESHOLD_INIT;

    void *tracker_new = SEE_REALLOC(heap->tracker, sizeof(see_object_t) * threshold_new);
    if (tracker_new)
    {
        heap->tracker = tracker_new;
        heap->gc_threshold = threshold_new;
    }
    heap->object_count = count_new;
    
    return 0;
}

see_heap_t 
see_heap_new(void)
{
    see_heap_t heap;
    
    heap = (see_heap_t)SEE_MALLOC(sizeof(see_heap_s));
    if (heap == NULL) return NULL;
    
    heap->object_count = 0;
    heap->gc_threshold = GC_THRESHOLD_INIT;
    heap->error_flag   = 0;
    
    if ((heap->tracker = (see_object_t *)SEE_MALLOC(sizeof(see_object_t) * heap->gc_threshold)) == NULL)
    {
        SEE_FREE(heap);
        return NULL;
    }
    
    return heap;
}

void
see_heap_free(see_heap_t heap)
{
    see_uintptr_t i;
    see_gc_header_t gc;
    for (i = 0; i < heap->object_count; ++ i)
    {
        gc = TO_GC_HEADER(heap->tracker[i]);
        /* Currently we free all objects including those protected */
        see_types[gc->type]->free(see_types[gc->type], heap->tracker[i]);
        SEE_FREE(gc);
    }

    SEE_FREE(heap->tracker);
    SEE_FREE(heap);
}

static const char *dummy_type_name(see_type_t self, see_object_t object) { return "DUMMY"; }
static void dummy_type_enumerate(see_type_t self, see_object_t object, void(*touch)(void *, see_object_t), void *priv) { }
static void dummy_type_free(see_type_t self, see_object_t object) { }
static see_type_s dummy_type = {
    .name      = dummy_type_name,
    .enumerate = dummy_type_enumerate,
    .free      = dummy_type_free,
};

static const char *external_type_name(see_type_t self, see_object_t object) {
    see_type_t type = (*(see_type_t *)object);
    return type->name(type, object);
}
static void external_type_enumerate(see_type_t self, see_object_t object, void(*touch)(void *, see_object_t), void *priv) {
    see_type_t type = (*(see_type_t *)object);
    return type->enumerate(type, object, touch, priv);
}
static void external_type_free(see_type_t self, see_object_t object) {
    see_type_t type = (*(see_type_t *)object);
    return type->free(type, object);
}
static see_type_s external_type = {
    .name      = external_type_name,
    .enumerate = external_type_enumerate,
    .free      = external_type_free,
};

void
see_object_sys_init(void)
{
    int i;
    for (i = 0; i < SEE_TYPE_TOTAL; ++ i)
        see_types[i] = &dummy_type;
    
    SEE_TYPE_ALIAS_REGISTER(SEE_TYPE_EXTERNAL, &external_type);
}
