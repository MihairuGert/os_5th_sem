COMPILER = gcc
LIB_NAME = coro
CFLAGS = -std=c99 -fsanitize=address -g

.PHONY: all clean

all:
	mkdir ../lib -p
	$(COMPILER) $(CFLAGS) -shared -fPIC -o ../lib/lib$(LIB_NAME).so coro_queue.c coroutine.c coro_map.c

clean:
	rm -f ../lib$(LIB_NAME).so
