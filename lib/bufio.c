#include "bufio.h"
#include "helpers.h"

//#define DEBUG

struct buf_t *buf_new(size_t capacity)
{
    struct buf_t * ptr = (struct buf_t*) malloc(sizeof(struct buf_t));
    ptr->capacity = capacity;
    ptr->size = 0;
    ptr->buf = (char *) malloc((sizeof(char))*capacity);

    return ptr;
}

void buf_free(struct buf_t *ptr)
{   
    #ifdef DEBUG
    if (ptr == NULL)
    {
        abort();
    }
    #endif

    free(ptr->buf);
    free(ptr);
}

size_t buf_capacity(struct buf_t *ptr)
{
    #ifdef DEBUG
    if (ptr == NULL)
    {
        abort();
    }
    #endif

    return ptr->capacity;
}

size_t buf_size(struct buf_t *ptr)
{
    #ifdef DEBUG
    if (ptr == NULL)
    {
        abort();
    }
    #endif

    return ptr->size;
}

ssize_t buf_fill(fd_t fd, struct buf_t *buf, size_t required)
{   
    #ifdef DEBUG
    if (buf == NULL)
    {
        abort();
    }
    if (buf->capacity < required)
    {
        abort();
    }
    #endif

    int cnt;
    while ((cnt = read(fd, (buf->buf + buf->size), buf->capacity - buf->size)) > 0)
    {
        buf->size += cnt;
        if (buf->size >= required)
        {
            break;
        }
    }
    if (cnt < 0)
    {
        return -1;
    }
    return buf->size;
}

size_t to_front(char *buf, int to_move, int count)
{   
    memmove(buf, buf + to_move, count - to_move);
    return count - to_move;
}

ssize_t buf_flush(fd_t fd, struct buf_t *buf, size_t required)
{
    #ifdef DEBUG
    if (buf == NULL)
    {
        abort();
    }
    #endif

    ssize_t total_count = 0;
    while (total_count < required)
    {
        int writed = write(fd, buf->buf + total_count, buf->size - total_count);
        if (!writed)
        {
            break;
        }
        if (writed < 0)
        {
            return -1;
        }
        total_count += writed;
    }
    buf->size = to_front(buf->buf, total_count, buf->size);
    return total_count;
}

