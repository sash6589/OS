#define _POSIX_SOURCE

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>

#include "helpers.h"
#include "bufio.h"

#define MAX_BUF 8192

struct buff {
    struct buf_t* tbuf1;
    struct buf_t* tbuf2;
    int flag1;
    int flag2;
};

struct buff buffs[127];
struct pollfd pollfds[256];

void close_all(int cnt)
{
    int i;
    for (i = 0; i < cnt; ++i)
    {
        close(pollfds[i].fd);
    }
}

int check(int n, int i, int j)
{
    if (((pollfds[i].revents & POLLERR) != 0) || ((pollfds[j].revents & POLLERR) != 0))
    {
        return 1;
    }
    if ((buffs[n].flag1) && (buffs[n].flag2))
    {
        return 1;
    }
    if (buffs[n].flag1 == 0)
    {
        if (((pollfds[i].revents & POLLOUT) != 0) && (buf_size(buffs[n].tbuf2) > 0) && (buf_flush(pollfds[i].fd, buffs[n].tbuf2, 1) == -1))
        {
            return 1;
        }

    }
    if (buffs[n].flag2 == 0)
    {
        if (((pollfds[j].revents & POLLOUT) != 0) && (buf_size(buffs[n].tbuf1) > 0) && (buf_flush(pollfds[j].fd, buffs[n].tbuf1, 1) == -1))
        {
            return 1;
        }
    }
    if (buffs[n].flag1 == 0)
    {
        if (((pollfds[j].revents & POLLIN) != 0) && (buf_size(buffs[n].tbuf2) < buf_capacity(buffs[n].tbuf2)))
        {
            size_t last = buf_size(buffs[n].tbuf2);
            if (buf_fill(pollfds[j].fd, buffs[n].tbuf2, last + 1) < 0)
            {
                return 1; 
            }

            if (last == buf_size(buffs[n].tbuf2))
            {
                buffs[n].flag1 = 1;
                shutdown(pollfds[j].fd, SHUT_RD);
            }
        }
    }
    if (buffs[n].flag2 == 0)
    {
        if (((pollfds[i].revents & POLLIN) != 0) && (buf_size(buffs[n].tbuf1) < buf_capacity(buffs[n].tbuf1)))
        {
            size_t last = buf_size(buffs[n].tbuf1);
            if (buf_fill(pollfds[i].fd, buffs[n].tbuf1, last + 1) < 0)
            {
                return 1; 
            }

            if (last == buf_size(buffs[n].tbuf1))
            {
                buffs[n].flag2 = 1;
                shutdown(pollfds[i].fd, SHUT_RD);
            }
        }
    }

    pollfds[i].events = 0;
    pollfds[j].events = 0;
    pollfds[i].revents = 0;
    pollfds[j].revents = 0;

    if (buffs[n].flag2 == 0)
    {
        if (buf_size(buffs[n].tbuf1) < buf_capacity(buffs[n].tbuf1))
        {
            pollfds[i].events = POLLIN;
        }
    }
    if (buffs[n].flag1 == 0)
    {
        if (buf_size(buffs[n].tbuf2) < buf_capacity(buffs[n].tbuf2))
        {
            pollfds[j].events = POLLIN;
        }
    }
    if ((buffs[n].flag2 == 0) && (buf_size(buffs[n].tbuf1) > 0))
    {
        pollfds[j].events |= POLLOUT;
    }
    if ((buffs[n].flag1 == 0) && (buf_size(buffs[n].tbuf2) > 0))
    {
        pollfds[i].events |= POLLOUT;
    }

    return 0;
}

void movebuffs(int i, int j)
{
    struct buff tmp;
    tmp.tbuf1 = buffs[i].tbuf1;
    tmp.tbuf2 = buffs[i].tbuf2;
    tmp.flag1 = buffs[i].flag1;
    tmp.flag2 = buffs[i].flag2;

    buffs[i].tbuf1 = buffs[j].tbuf1;
    buffs[i].tbuf2 = buffs[j].tbuf2;
    buffs[i].flag1 = buffs[j].flag1;
    buffs[i].flag2 = buffs[j].flag2;

    buffs[j].tbuf1 = tmp.tbuf1;
    buffs[j].tbuf2 = tmp.tbuf2;
    buffs[j].flag1 = tmp.flag1;
    buffs[j].flag2 = tmp.flag2;
}

void movepollfds(int i, int j)
{
    struct pollfd tmp;

    tmp.fd = pollfds[i].fd;
    tmp.events = pollfds[i].events;
    tmp.revents = pollfds[i].revents;

    pollfds[i].fd = pollfds[j].fd;
    pollfds[i].events = pollfds[j].events;
    pollfds[i].revents = pollfds[j].revents;

    pollfds[j].fd = tmp.fd;
    pollfds[j].events = tmp.events;
    pollfds[j].revents = tmp.revents;
}

void start_server(int sfd1, int sfd2)
{
    int i;
    int count = 0;
    int flag = 0;

    while (1)
    {
        if (count == 127)
        {
            pollfds[0].events = 0;
            pollfds[1].events = 0;
        }
        else
        {
            if (flag == 0)
            {
                pollfds[0].events = POLLIN;
                pollfds[1].events = 0;
            }
            else
            {
                pollfds[0].events = 0;
                pollfds[1].events = POLLIN;
            }
        }

        if ((poll(pollfds, count * 2 + 2, -1) == -1) && (errno != EINTR))
        {
            close_all(count * 2 + 2);
            exit(EXIT_FAILURE);
        }

        for (i = 0; i < count; ++i)
        {
            if (check(i, i * 2 + 2, i * 2 + 3))
            {
                --count;
                close(pollfds[2*i + 2].fd);
                close(pollfds[2*i + 3].fd);
                movebuffs(i, count);
                movepollfds(i * 2 + 2, count * 2 + 2);
                movepollfds(i * 2 + 3, count * 2 + 3);
            }
        }

        if (count < 127)
        {
            if ((flag) && ((pollfds[1].revents & POLLIN) != 0))
            {
                struct sockaddr ad;
                socklen_t len = sizeof(struct sockaddr);

                int fd1 = accept(sfd1, &ad, &len);
                int fd2 = accept(sfd2, &ad, &len);

                if ((fd1 < 0) || (fd2 < 0))
                {
                    if (fd1 != -1)
                    {
                        close(fd1);
                    }
                    if (fd2 != -1)
                    {
                        close(fd2);
                    }
                }
                else
                {
                    buffs[count].flag1 = 0;
                    buffs[count].flag2 = 0;
                    buf_set(buffs[count].tbuf1, 0);
                    buf_set(buffs[count].tbuf2, 0);
                    pollfds[2*count + 2].fd = fd1;
                    pollfds[2*count + 2].events = POLLIN | POLLERR;
                    pollfds[2*count + 2].revents = 0;
                    pollfds[2*count + 3].fd = fd2;
                    pollfds[2*count + 3].events = POLLIN | POLLERR;
                    pollfds[2*count + 3].revents = 0;
                    ++count;
                }
                flag = 0;
                pollfds[0].revents = 0;
                pollfds[1].revents = 0;
            }
            if ((!flag) && ((pollfds[0].revents & POLLIN) != 0))
            {
                flag = 1;
            }
        }
    }
}

void init_info(struct addrinfo *info)
{
    memset(info, 0, sizeof(struct addrinfo));
    info->ai_family = AF_INET;
    info->ai_socktype = SOCK_STREAM;
    info->ai_protocol = IPPROTO_TCP;
    info->ai_flags = AI_PASSIVE;
}

int main(int argc, char *argv[])
{
    int i;
    
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



    if ((listen(sfd1, 127) < 0) || (listen(sfd2, 127) < 0))
    {
        return EXIT_FAILURE;
    }

    for (i = 0; i < 127; ++i)
    {
        buffs[i].tbuf1 = buf_new(MAX_BUF);
        buffs[i].tbuf2 = buf_new(MAX_BUF);
    }

    pollfds[0].fd = sfd1;
    pollfds[1].fd = sfd2;
    pollfds[0].revents = 0;
    pollfds[1].revents = 0;

    if ((listen(sfd1, 127) < 0) || (listen(sfd2, 127) < 0))
    {
        return EXIT_FAILURE;
    }

    start_server(sfd1, sfd2);

}


