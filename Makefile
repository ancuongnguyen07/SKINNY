CFLAGS=-g -O0 -Wall -Wextra

all: driver_tk3

driver_tk3: driver_tk3.c tk3.o skinny.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f driver_tk3 tk3.o skinny.o
