CC = gcc
CFLAGS = -Wall
OBJS = f403tech_drv_test.o

all: f403tech_drv_test

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)

fbtest: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f rbcfg *.o
