#define _POSIX_SOURCE

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "helpers.h"
#include "bufio.h"

#define MAX_BUF 8192

void close_handler(int code)
{
    code = 0;
    while (1) {
        if ((wait(NULL)) == -1 && (errno == ECHILD)) 
        {
            break;
        }
    }
    exit(EXIT_SUCCESS);
}

void init_sig(struct sigaction *sig_action, sigset_t *sig_set)
{
    memset(sig_action, 0, sizeof(struct sigaction));
    sigemptyset(sig_set);
    sigaddset(sig_set, SIGINT);
    sig_action->sa_mask = (*sig_set);
    sig_action->sa_handler = close_handler;
}

void init_info(struct addrinfo *info)
{
    memset(info, 0, sizeof(struct addrinfo));
    info->ai_family = AF_INET;
    info->ai_socktype = SOCK_STREAM;
    info->ai_protocol = IPPROTO_TCP;
    info->ai_flags = AI_PASSIVE;
}

void start_server(int sfd1, int sfd2)
{   
    struct sockaddr_in addr_in1;
    socklen_t len1 = sizeof(struct sockaddr_in);

    struct sockaddr_in addr_in2;
    socklen_t len2 = sizeof(struct sockaddr_in);

    while (1)
    {
        int fd1 = accept(sfd1, (struct sockaddr *) &addr_in1, &len1);
            int fd2 = accept(sfd2, (struct sockaddr *) &addr_in2, &len2);


        pid_t id1 = fork();

        if (id1 < 0)
        {
            exit(EXIT_FAILURE);
        }

        if (id1 == 0)
        {
            struct buf_t *tbuf = buf_new(MAX_BUF);
            while (1)
            {
                write_(fd1, "\n< ", 3);
                int cnt = buf_fill(fd1, tbuf, 1);
                write_(fd2, "\n> ", 3);
                buf_flush(fd2, tbuf, cnt);
                write_(fd2, "\n< ", 3);
                if (cnt == 0)
                {
                    break;
                }
            }
            close(fd1);
            close(fd2);
            return;
        }

        pid_t id2 = fork();

        if (id2 < 0)
        {
            exit(EXIT_FAILURE);
        }

        if (id2 == 0)
        {
            struct buf_t *tbuf = buf_new(MAX_BUF);
            while (1)
            {
                write_(fd2, "\n< ", 3);
                int cnt = buf_fill(fd2, tbuf, 1);
                write_(fd1, "\n> ", 3);
                buf_flush(fd1, tbuf, cnt);
                write_(fd1, "\n< ", 3);
                if (cnt == 0)
                {
                    break;
                }
            }
            close(fd1);
            close(fd2);
        }
        close(fd1);
        close(fd2);
    }
}

int main(int argc, char *argv[])
{
    struct sigaction sig_action;
    sigset_t sig_set;
    init_sig(&sig_action, &sig_set);
    sigaction(SIGINT, &sig_action, 0);

    struct addrinfo info;
    init_info(&info);

    struct addrinfo *ans;

    if (getaddrinfo("localhost", argv[1], &info, &ans) < 0)
    {
        perror("getaddrinfo");
        return EXIT_FAILURE;
    }

    int sfd1 = socket(ans->ai_family, ans->ai_socktype, ans->ai_protocol);
    int num = 1;
    if (setsockopt(sfd1, SOL_SOCKET, SO_REUSEADDR, &num, sizeof(int)) < 0)
    {
        return EXIT_FAILURE;
    }

    if (bind(sfd1, ans->ai_addr, ans->ai_addrlen) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(ans);

    struct addrinfo *ans1;

    if (getaddrinfo("localhost", argv[2], &info, &ans1) < 0)
    {
        perror("getaddrinfo");
        return EXIT_FAILURE;
        return EXIT_FAILURE;
    }

    int sfd2 = socket(ans1->ai_family, ans1->ai_socktype, ans1->ai_protocol);
    if (setsockopt(sfd2, SOL_SOCKET, SO_REUSEADDR, &num, sizeof(int)) < 0)
    {
        return EXIT_FAILURE;
    }

    if (bind(sfd2, ans1->ai_addr, ans1->ai_addrlen) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }


    freeaddrinfo(ans1);

    if ((listen(sfd1, 1) < 0) || (listen(sfd2, 1)))
    {
        return EXIT_FAILURE;
    }

    start_server(sfd1, sfd2);
}
