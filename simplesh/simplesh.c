#define _POSIX_SOURCE

#include <string.h>

#include "helpers.h"
#include "bufio.h"

#define BUF_SIZE 8192

char buf[BUF_SIZE];

char** split(char *s, const char *d)
{
    int cnt = 0;
    char **ans = NULL;
    char *ptr = (char *) strtok(s, d);

    while (ptr != NULL)
    {
        ++cnt;
        ans = (char**) realloc(ans, cnt * sizeof(char *));
        ans[cnt - 1] = ptr;
        ptr = (char *) strtok(NULL, d);
    }
    ++cnt;
    ans = (char**) realloc(ans, cnt * sizeof(char *));
    ans[cnt - 1] = NULL;
    return ans;
}

void sig_handler(int code)
{
    code = 0;
}

int main(int argc, char *argv[])
{
    
    struct sigaction sig_action;
    memset(&sig_action, 0, sizeof(struct sigaction));
    sigset_t sig_set;
    sigemptyset(&sig_set);
    sigaddset(&sig_set, SIGINT);
    sig_action.sa_mask = sig_set;
    sig_action.sa_handler = sig_handler;
    sigaction(SIGINT, &sig_action, 0);

    struct buf_t *tbuf = buf_new(BUF_SIZE);

    while (1)
    {
        if (write_(STDOUT_FILENO, "$", 1) < 0)
        {
            return EXIT_FAILURE;
        }

        int cnt = buf_getline(STDIN_FILENO, tbuf, buf);
        
        if ((cnt == 0) && (tbuf->stop))
        {
            exit(EXIT_SUCCESS);
        }
        
        if (cnt > 0)
        {
            char** programs = split(buf, "|\n");

            int i = 0;

            struct execargs_t **exec_programs = NULL;
            while (programs[i] != NULL)
            {
                char** program = split(programs[i], " ");
                exec_programs = (struct execargs_t **) realloc(exec_programs, sizeof(struct execargs_t *) * (i + 1));
                exec_programs[i] = create_execargs(program);
                ++i;
            }
            runpiped(exec_programs, i);
            memset(buf, 0, BUF_SIZE);
        }
    }
}