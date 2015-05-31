#include "bufio.h"
#include "helpers.h"

//#define DEBUG

struct buf_t *buf_new(size_t capacity)
{
    struct buf_t * ptr = (struct buf_t*) malloc(sizeof(struct buf_t));
    ptr->capacity = capacity;
    ptr->size = 0;
    ptr->buf = (char *) malloc((sizeof(char))*capacity);
    ptr->stop = 0;

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

ssize_t buf_getline(int fd, struct buf_t *buf, char * dest) {
    
    #ifdef DEBUG
    if (buf == NULL)
    {
        abort();
    }
    #endif

    int i;
    int length = 0;
    int cnt;
    while (1)
    {
        for (i = 0; i < buf->size; ++i)
        {
            if (buf->buf[i] == '\n') 
            {
                memmove(dest + length, buf->buf, i + 1);
                buf->size = to_front(buf->buf, i + 1, buf->size);
                return length + i + 1;
            }
        }
        memmove(dest + length, buf->buf, buf->size);
        buf->size = to_front(buf->buf, buf->size, buf->size);
        length += buf->size;
        cnt = buf_fill(fd, buf, 1);
        if (cnt == 0)
        {
            buf->stop = 1;
            break;
        }
    }
    if (cnt < 0)
    {
        return -1;
    }
    return length;    
}