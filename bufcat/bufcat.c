#include "bufio.h"

#define BUF_SIZE 4096

#define TEST

int main(int argc, char *argv[])
{

    struct buf_t *buf = buf_new(BUF_SIZE);

    int cnt;
    while ((cnt = buf_fill(STDIN_FILENO, buf, buf_capacity(buf))) > 0)
    {
        ssize_t writed = buf_flush(STDOUT_FILENO, buf, cnt);
        if (writed < 0)
        {
            buf_free(buf);
            return EXIT_FAILURE;
        }
    }
    if (cnt < 0)
    {
        buf_flush(STDOUT_FILENO, buf, buf_size(buf));
        buf_free(buf);
        return EXIT_FAILURE;
    }

    buf_free(buf);
    return 0;
}