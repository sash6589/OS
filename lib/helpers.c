#include "helpers.h"


ssize_t read_(int fd, void *buf, size_t count)
{   
    ssize_t total_count = 0;
    int cnt = 0;

    while ((cnt = read(fd, buf + total_count, count - total_count)) > 0)
    {
        total_count += cnt;
    }

    if (cnt < -1)
    {
        return EXIT_FAILURE;
    }

    return total_count;
}

ssize_t write_(int fd, const void *buf, size_t count)
{
    ssize_t total_count = 0;
    while (total_count < count)
    {
        int writed = write(fd, buf + total_count, count - total_count);
        if (!writed)
        {
            break;
        }
        if (writed < 0)
        {
            return EXIT_FAILURE;
        }
        total_count += writed;
    } 
    return total_count;

}

ssize_t read_until(int fd, void * buf, size_t count, char delimiter)
{
    ssize_t total_count = 0;
    ssize_t cnt = 0;

    while ((cnt = read(fd, buf + total_count, count - total_count)) > 0)
    {   
        int has_delimiter = 0;
        int i;
        for (i = total_count; i < total_count + cnt; ++i)
        {
            if (((char *) buf)[i] == delimiter)
            {
                has_delimiter = 1;
            }
        }
        total_count += cnt;
        if (has_delimiter)
        {
            break;
        }
    }

    if (cnt < 0)
    {
        return EXIT_FAILURE;
    }

    return total_count; 
}

int spawn(const char * file, char * const argv[])
{ 
    pid_t pid = fork();
    if (pid < 0)
    {
        return EXIT_FAILURE;
    }
    if (pid)
    {
        int return_status;
        pid_t v = wait(&return_status);
        if (v < 0)
        {
            return EXIT_FAILURE;
        }
        if (!(WIFEXITED(return_status)))
        {
            return EXIT_FAILURE;
        }
        else
        {
            return WEXITSTATUS(return_status);
        }
    }
    else
    {
        if (execvp(file, argv) < 0)
        {
            return EXIT_FAILURE;
        }
    }
}

