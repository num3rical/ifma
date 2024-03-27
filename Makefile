CFLAGS = -g -Wall
LDFLAGS = -g

LIBS = -lpthread -ldl

TARGET = ifma

ifeq ($(PREFIX),)
	PREFIX := /usr/local/bin
endif

all: $(TARGET)

$(TARGET): ifma.o sqlite3.o
	$(CXX) $(LDFLAGS) $(LIBS) $^ -o $@

sqlite3.o: deps/sqlite3.c deps/sqlite3.h
	$(CC) $(CFLAGS) -Ideps -c -o $@ $<

install:
	install $(TARGET) $(DESTDIR)$(PREFIX)

.PHONY: clean

clean:
	$(RM) $(TARGET) *.o

