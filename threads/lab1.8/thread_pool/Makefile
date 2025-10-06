COMPILER = gcc
LIB_NAME = thread_pool
CFLAGS = -std=c99 -fsanitize=address -g

.PHONY: all clean

all:
	$(COMPILER) $(CFLAGS) -shared -fPIC -o ../lib/lib$(LIB_NAME).so thread_pool.c

clean:
	rm -f ../lib$(LIB_NAME).so
	