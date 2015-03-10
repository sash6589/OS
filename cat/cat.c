#include "helpers.h"

#define BUF_SIZE 4096

int main(int argc, char* argv[])
{	
	int cnt = 0;
	char buf[BUF_SIZE];
	while ((cnt = read_(STDIN_FILENO, buf, BUF_SIZE)) > 0)
	{
		int writed = write_(STDOUT_FILENO, buf, cnt);
		if (writed < 0)
		{
			exit(EXIT_FAILURE);
		}
	}
	if (cnt < 0)
	{
		exit(EXIT_FAILURE);
	}
}