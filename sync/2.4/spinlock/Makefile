COMPILER = gcc
LIB_NAME = spinlock
CFLAGS = -std=c99 -fsanitize=address -fsanitize=leak -g -Wall -Wextra -Werror

.PHONY: all clean

all:
	mkdir ../lib -p
	$(COMPILER) $(CFLAGS) -shared -fPIC -o ../lib/lib$(LIB_NAME).so spinlock.c

clean:
	rm -f ../lib$(LIB_NAME).so
