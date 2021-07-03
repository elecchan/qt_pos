#include <stdio.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#define MYLEDS_LED0_ON 	0
#define MYLEDS_LED0_OFF 	1
#define MYLEDS_LED1_ON 	2
#define MYLEDS_LED1_OFF 	3
#define MYLEDS_LED2_ON 	4
#define MYLEDS_LED2_OFF 	5
#define MYLEDS_LED3_ON 	6
#define MYLEDS_LED3_OFF 	7
#define MYLEDS_LED4_ON 	8
#define MYLEDS_LED4_OFF 	9

/*
**  ledtest <dev> <on|off>
**/

void print_usage(char *file)
{
	printf("Usage:\n");
	printf("%s <dev> <on|off>\n",file);
	printf("eg. \n");
	printf("%s led0 on\n", file);
	printf("%s led0 off\n", file);
	printf("%s led1 on\n", file);
	printf("%s led1 off\n", file);
	printf("%s led2 on\n", file);
	printf("%s led2 off\n", file);
	printf("%s led3 on\n", file);
	printf("%s led3 off\n", file);
	printf("%s led4 on\n", file);
	printf("%s led4 off\n", file);
}

int main(int argc, char **argv)
{
	int fd;

	if (argc != 3)
	{
		print_usage(argv[0]);
		return 0;
	}

	/* 1.´ò¿ªÉè±¸½Úµã */
	fd = open("/dev/myleds", O_RDWR | O_NONBLOCK);
	if (fd < 0)
	{
		printf("can't open!\n");
		return -1;
	}

	/* 2.¸ù¾Ý²ÎÊý²»Í¬£¬¿ØÖÆLEDs */
	if(!strcmp("led0", argv[1]))
	{
		if (!strcmp("on", argv[2]))
		{
			// ÁÁµÆ
			ioctl(fd, MYLEDS_LED0_ON);
		}
		else if (!strcmp("off", argv[2]))
		{
			// ÃðµÆ
			ioctl(fd, MYLEDS_LED0_OFF);
		}
		else
		{
			print_usage(argv[0]);
			return 0;
		}
	}
	else if(!strcmp("led1", argv[1]))
	{
		if (!strcmp("on", argv[2]))
		{
			// ÁÁµÆ
			ioctl(fd, MYLEDS_LED1_ON);
		}
		else if (!strcmp("off", argv[2]))
		{
			// ÃðµÆ
			ioctl(fd, MYLEDS_LED1_OFF);
		}
		else
		{
			print_usage(argv[0]);
			return 0;
		}
	}
	else if(!strcmp("led2", argv[1]))
	{
		if (!strcmp("on", argv[2]))
		{
			// ÁÁµÆ
			ioctl(fd, MYLEDS_LED2_ON);
		}
		else if (!strcmp("off", argv[2]))
		{
			// ÃðµÆ
			ioctl(fd, MYLEDS_LED2_OFF);
		}
		else
		{
			print_usage(argv[0]);
			return 0;
		}
	}
	else if(!strcmp("led3", argv[1]))
	{
		if (!strcmp("on", argv[2]))
		{
			// ÁÁµÆ
			ioctl(fd, MYLEDS_LED3_ON);
		}
		else if (!strcmp("off", argv[2]))
		{
			// ÃðµÆ
			ioctl(fd, MYLEDS_LED3_OFF);
		}
		else
		{
			print_usage(argv[0]);
			return 0;
		}
	}
	else if(!strcmp("led4", argv[1]))
	{
		if (!strcmp("on", argv[2]))
		{
			// ÁÁµÆ
			ioctl(fd, MYLEDS_LED4_ON);
		}
		else if (!strcmp("off", argv[2]))
		{
			// ÃðµÆ
			ioctl(fd, MYLEDS_LED4_OFF);
		}
		else
		{
			print_usage(argv[0]);
			return 0;
		}
	}
	else
	{
		print_usage(argv[0]);
		return 0;
	}

	return 0;
}

