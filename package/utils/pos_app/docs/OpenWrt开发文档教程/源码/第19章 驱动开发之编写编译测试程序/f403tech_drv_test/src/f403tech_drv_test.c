
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	int fd;
	char val;

	fd = open("/dev/f403tech", O_RDWR);
	if (fd < 0)
	{
		printf("Can't open /dev/f403tech\n");
	}

	read(fd, &val, 1);

	write(fd, &val, 1);

	return 0;
}

