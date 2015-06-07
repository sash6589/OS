#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

typedef int fd_t;

struct buf_t
{
    size_t capacity;
    size_t size;
    char *buf;
    int stop;
};

struct buf_t *buf_new(size_t capacity);
void buf_free(struct buf_t *);
size_t buf_capacity(struct buf_t *);
size_t buf_size(struct buf_t *);
ssize_t buf_fill(fd_t fd, struct buf_t *buf, size_t required);
ssize_t buf_flush(fd_t fd, struct buf_t *buf, size_t required);
ssize_t buf_getline(int fd, struct buf_t *buf, char * dest);
void buf_set(struct buf_t *ptr, size_t n);