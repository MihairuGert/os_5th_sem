COMPILER = gcc
LIB_NAME = hashtable
CFLAGS = -fsanitize=address -fsanitize=leak -g -Wall -Wextra -Werror

.PHONY: all clean

all:
	$(COMPILER) $(CFLAGS) -shared -fPIC -o ../lib$(LIB_NAME).so hash_table.c

clean:
	rm -f ../lib$(LIB_NAME).so
	