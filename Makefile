CC = gcc
CFLAGS = -fPIC -Wall -Wextra -g
LDFLAGS = -shared

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = libosmem.so

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

clean:
	-rm -f $(TARGET)
	-rm -f $(OBJS)
