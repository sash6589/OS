#define _POSIX_SOURCE

#include <string.h>
#include <errno.h>

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
    return 0;
}

struct execargs_t* create_execargs(char * args[])
{
    struct execargs_t *ans = (struct execargs_t *) malloc(sizeof(struct execargs_t));

    if (ans == NULL)
    {
        exit(EXIT_FAILURE);
    }

    ans->prog_args = args;
    ans->prog_name = args[0];

    return ans;
}

int exec(struct execargs_t* args)
{
    return execvp(args->prog_name, args->prog_args);
}

pid_t *pids;
size_t n_pids;

void kill_all()
{
    for (size_t i = 0; i < n_pids; ++i)
    {
        if (pids[i] != -1)
        {
            kill(pids[i], SIGKILL);
        }
    }
    n_pids = 0;
}

void kill_handler(int code)
{
    code = 0;
    kill_all();
}

void sig_init(struct sigaction *sig_action, sigset_t *sig_set)
{
    memset(sig_action, 0, sizeof(struct sigaction));
    sigemptyset(sig_set);
    sigaddset(sig_set, SIGINT);
    sig_action->sa_mask = (*sig_set);
    sig_action->sa_handler = kill_handler;

}

void exit_handler(int code)
{
    code = 0;
    exit(EXIT_FAILURE);
}

void other_sig_init(struct sigaction *other_sig_action, sigset_t *other_sig_set)
{
    memset(other_sig_action, 0, sizeof(struct sigaction));
    sigemptyset(other_sig_set);
    sigaddset(other_sig_set, SIGPIPE);
    other_sig_action->sa_mask = (*other_sig_set);
    other_sig_action->sa_handler = exit_handler;

}

int runpiped(struct execargs_t** programs, size_t n)
{  
    if (!n)
    {
        return 0;
    }

    pids = (pid_t *) malloc (sizeof(pid_t) * n);
    if (pids == NULL)
    {
        return EXIT_FAILURE;
    }
    n_pids = 0;

    for (int i = 0; i < n; ++i)
    {
        pids[i] = -1;
    }

    struct sigaction sig_action;
    sigset_t sig_set;
    sig_init(&sig_action, &sig_set);
    sigaction(SIGINT, &sig_action, NULL);

    int fds[n - 1][2];
    for (int i = 0; i < n - 1; ++i)
    {
        if (pipe(fds[i]) == -1)
        {
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < n; ++i)
    {
        pid_t id = fork();

        if (id < 0)
        {
            kill_all();
            for (int i = 0; i < n - 1; ++i)
            {
                if (close(fds[i][0]) == -1)
                {
                    return EXIT_FAILURE;
                }
                if (close(fds[i][1]) == -1)
                {
                    return EXIT_FAILURE;
                }
            }
            return EXIT_FAILURE;
        }

        if (id)
        {
            pids[n_pids] = id;
            ++n_pids;
        }
        else
        {
            if (i > 0)
            {
                if (dup2(fds[i - 1][0], 0) == -1)
                {
                    exit(EXIT_FAILURE);
                }
            }

            if (i < n - 1)
            {
                if (dup2(fds[i][1], 1) == -1)
                {
                    exit(EXIT_FAILURE);
                }
            }

            for (int j = 0; j < n - 1; ++j)
            {
                if (j != i - 1)
                {
                    if (close(fds[j][0]) == -1)
                    {
                        exit(EXIT_FAILURE);
                    }
                }
                if (j != i)
                {
                    if (close(fds[j][1]) == -1)
                    {
                        exit(EXIT_FAILURE);
                    }
                }
            }

            struct sigaction other_sig_action;
            sigset_t other_sig_set;
            other_sig_init(&other_sig_action, &other_sig_set);
            sigaction(SIGPIPE, &other_sig_action, NULL);
            exec(programs[i]);
            printf("%d\n", i);
        }
    }

    while (1) {
        if ((wait(NULL) == -1) && (errno == ECHILD))
        {
            break;
        }
    }

    for (int i = 0; i < n - 1; ++i)
    {
        if (close(fds[i][0]) == -1)
        {
            return EXIT_FAILURE;
        }
        if (close(fds[i][1]) == -1)
        {
            return EXIT_FAILURE;
        }
    }

    return 0;
}

