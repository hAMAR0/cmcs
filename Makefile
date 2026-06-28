CC      = gcc
CFLAGS  = -Wall -Wextra -g

SRC     = main.c network/server.c network/requests.c protocol/packets.c
OBJ     = $(SRC:.c=.o)
TARGET  = main

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)
