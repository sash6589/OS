#include <string.h>

#include "helpers.h"

#define BUF_SIZE 8192

int to_front(char *buf, int to_move, int count)
{   
    memmove(buf, buf + to_move, count - to_move);
    return count - to_move;
}

int main(int argc, char const *argv[])
{   

    char const *args[argc + 1];
    int args_len = argc - 1;
    int i;
    for (i = 1; i < argc; ++i)
    {
        args[i - 1] = argv[i];
    }

    char buf[BUF_SIZE];
    char arg[BUF_SIZE];
    ssize_t cnt;
    int size = 0;

    while ((cnt = read_until(STDIN_FILENO, buf + size, BUF_SIZE - size, '\n')) > 0)
    {   
        int word_cnt = cnt + size;
        while (1)
        {
            int flag = 0;
            for (i = 0; i < word_cnt; ++i)
            {
                if (buf[i] == '\n')
                {
                    memcpy(arg, buf, i);
                    arg[i] = 0;
                    args[args_len] = arg;
                    args[args_len + 1] = NULL;
                    int res = spawn(args[0], args);
                    if (!res)
                    {   
                        arg[i] = '\n';
                        ssize_t writed = write_(STDOUT_FILENO, arg, i + 1);
                        if (writed < 0)
                        {
                            exit(EXIT_FAILURE);
                        }
                    }
                    word_cnt = to_front(buf, i + 1, word_cnt);
                    flag = 1;
                    break;
                }
            }
            if (!flag)
            {
                break;
            }
        }
        size = word_cnt;
    }
    if (cnt < 0)
    {
        exit(EXIT_FAILURE);
    }

    if (size > 0)
    {
        memcpy(arg, buf, size);
        arg[size] = 0;
        args[args_len] = arg;
        args[args_len + 1] = NULL;
        int res = spawn(args[0], args);
        if (!res)
        {
            ssize_t writed = write_(STDOUT_FILENO, arg, i + 1);
            if (writed < 0)
            {
                exit(EXIT_FAILURE);
            }
        }
    }
    return 0;
}