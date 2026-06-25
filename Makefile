CC      = gcc
CFLAGS  = -Wall -Wextra -g

SRC     = main.c network/server.c protocol/packets.c
OBJ     = $(SRC:.c=.o)
TARGET  = main

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)
