COMPILER = gcc
LIB_NAME = thread_pool
CFLAGS = -fsanitize=address -fsanitize=leak -g -Wall -Wextra -Werror

.PHONY: all clean

all:
	$(COMPILER) $(CFLAGS) -shared -fPIC -o ../lib$(LIB_NAME).so thread_pool.c

clean:
	rm -f ../lib$(LIB_NAME).so
	