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

char *in;

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

void start_server(int sfd)
{   
    struct sockaddr_in addr_in;
    socklen_t len = sizeof(struct sockaddr_in);
    
    while (1)
    {   
        int fd = accept(sfd, (struct sockaddr *) &addr_in, &len);
        pid_t id = fork();
        if (id < 0)
        {
            exit(EXIT_FAILURE);
        }
        if (id == 0)
        {
            struct buf_t *tbuf = buf_new(MAX_BUF);
            int in_file = open(in, O_RDONLY);
            while (1)
            {
                int cnt = buf_fill(in_file, tbuf, 1);
                if (cnt == 0)
                {
                    break;
                }
                buf_flush(fd, tbuf, cnt);
            }
            close(fd);
            close(in_file);
            return;
        }
        close(fd);
    }
    close(sfd);
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
        return EXIT_FAILURE;
    }

    int sfd = socket(ans->ai_family, ans->ai_socktype, ans->ai_protocol);
    int num = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &num, sizeof(int)) < 0)
    {
        return EXIT_FAILURE;
    }

    bind(sfd, ans->ai_addr, ans->ai_addrlen);

    freeaddrinfo(ans);

    if (listen(sfd, 1) < 0)
    {
        return EXIT_FAILURE;
    }

    in = argv[2];

    start_server(sfd);
}