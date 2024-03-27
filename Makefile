OBJS = ifma.cpp

CC = g++
CFLAGS = -g -Wall

LFLAGS = -lsqlite3

TARGET = ifma

ifeq ($(PREFIX),)
	PREFIX := /usr/local/bin
endif

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(LFLAGS) -o $@

install:
	install $(TARGET) $(DESTDIR)$(PREFIX)

clean:
	$(RM) $(TARGET)

