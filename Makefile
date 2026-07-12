CC = gcc

CFLAGS = -Wall -Wextra -g -Iinclude -Ilib
LDFLAGS = -pthread

TARGET = server

SRC = server.c \
      include/functions.c \
      lib/proxy_parse.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean