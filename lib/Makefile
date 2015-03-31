all: libbufio.so libhelpers.so
libhelpers.so: helpers.c
	c99 -fPIC -Wall -shared helpers.c -o libhelpers.so
libbufio.so: bufio.c
	c99 -fPIC -Wall -shared bufio.c -o libbufio.so