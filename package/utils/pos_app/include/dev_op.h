#ifndef _DEV_OP_H_HEADER  
#define _DEV_OP_H_HEADER  
#include <termios.h>
/*******************************************************************/
#define TTY_DEV "/dev/ttyS0"
int open_tty(int baud,int databit,int parity,int stopbit);
int close_tty(int fd);
speed_t tty_getBaudrate(int baudrate);
int tty_config(int fd,int baud,int databit,int parity,int stopbit);
int tty_send(int fd, char *data, int datalen);
int tty_recv(int fd, char *data, int datalen);
void init_dev(void);
/*******************************************************************/
#define INT_DEV "/dev/gpio_int"
//#define UP_KEY    1
//#define DOWN_KEY  2
#define UP_KEY    24
#define DOWN_KEY  25

#define UP 			 1
#define DOWN 		 2
#define PAUSE		 3
#define EXCEPTION    4

#define CMD_FLAG  'a'
#define USERSPACE_RD		1
#define USERSPACE_WR		2
typedef struct _userspace_data {
    int set_count;
	int get_count;
	unsigned char status;
} userspace_data;
int get_int_count(void);
int set_int_count(int data);

/*******************************************************************/
#define KEY_DEV "/dev/gpio_key"
int key_press(void);
/*******************************************************************/

#endif
