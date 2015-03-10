#include "helpers.h"

ssize_t read_(int fd, void *buf, size_t count)
{	
	char help_buf[count];
	ssize_t total_count = 0;
	int cnt = 0;

	while ((cnt = read(fd, help_buf, count)) > 0)
	{
		int to_copy;
		
		if (cnt < count - total_count)
		{
			to_copy = cnt;
		}
		else
		{
			to_copy = count - total_count;
		}

		if (!to_copy)
		{
			break;
		}
		memcpy(buf + total_count, help_buf, to_copy);
		total_count += to_copy;
	}

	if (cnt == -1)
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

