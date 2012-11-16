#include <see/vm/vm.h>

#include <stdio.h>

struct simple_stream_s
{
    FILE *file;
    int   buf;
};

typedef struct simple_stream_s  simple_stream_s;
typedef struct simple_stream_s *simple_stream_t;

void simple_stream_open(simple_stream_t stream, FILE *file);
void simple_stream_close(simple_stream_t stream);
int  simple_stream_in(simple_stream_t stream, int advance);

#define BUF_END   (-1)
#define BUF_ERROR (-2)
#define BUF_EMPTY (-3)

void
simple_stream_open(simple_stream_t stream, FILE *file)
{
    stream->file = file;
    stream->buf  = file == NULL ? BUF_ERROR : BUF_EMPTY;
}

void
simple_stream_close(simple_stream_t stream)
{
    if (stream->file != NULL) fclose(stream->file);
    stream->file = NULL;
    stream->buf  = BUF_ERROR;
}

int
simple_stream_in(simple_stream_t stream, int advance)
{
    int r;
    if (advance)
    {
        if (stream->buf == BUF_EMPTY)
            r = fgetc(stream->file);
        else
        {
            r = stream->buf;
            if (stream->buf != BUF_ERROR)
                stream->buf = BUF_EMPTY;
        }
    }
    else
    {
        if (stream->buf == BUF_EMPTY)
        {
            stream->buf = fgetc(stream->file);
            if (stream->buf < 0) stream->buf = BUF_END;
        }
        
        r = stream->buf;
    }
    return r;
}

int
main(void)
{
    see_object_sys_init();

    see_heap_t heap = see_heap_new();

    simple_stream_s ss;
    simple_stream_open(&ss, fopen("input", "r"));

    see_vm_prog_t prog = see_vm_prog_parse_bin(heap, (see_io_char_stream_in_f)simple_stream_in, &ss);
    if (prog == NULL)
    {
        printf("error while parse input\n");
        return 1;
    }

    see_vm_t vm = see_vm_new(heap, prog);
    see_vm_exception_s ex;
    see_vm_run(vm, &ex);

    printf("ex no %d\n", ex.type);
    

    return 0;
}
