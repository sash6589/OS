#ifndef HELPERS_H 
#define HELPERS_H 

#define _POSIX_SOURCE
 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

ssize_t read_(int fd, void *buf, size_t count);
ssize_t write_(int fd, const void *buf, size_t count);
ssize_t read_until(int fd, void * buf, size_t count, char delimiter);
int spawn(const char * file, char * const argv []);

struct execargs_t {
    char** prog_args;
    char* prog_name;
};

struct execargs_t *create_execargs(char * args[]);
int exec(struct execargs_t* args);
int runpiped(struct execargs_t** programs, size_t n);

/* HELPERS_H */
#endif
