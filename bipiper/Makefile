all: forking polling

forking: forking.c
	c99 -Wall forking.c -o forking -L ../lib -lhelpers -lbufio -Wl,-rpath=../lib -I ../lib

forking: polling.c
	c99 -Wall polling.c -o polling -L ../lib -lhelpers -lbufio -Wl,-rpath=../lib -I ../lib

