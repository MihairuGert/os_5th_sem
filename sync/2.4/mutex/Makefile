COMPILER = gcc
LIB_NAME = mutex
CFLAGS = -std=c99 -fsanitize=address -fsanitize=leak -g -Wall -Wextra -Werror

.PHONY: all clean

all:
	mkdir ../lib -p
	$(COMPILER) $(CFLAGS) -shared -fPIC -o ../lib/lib$(LIB_NAME).so mutex.c

clean:
	rm -f ../lib$(LIB_NAME).so
