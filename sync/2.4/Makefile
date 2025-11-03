COMPILER = gcc
TARGET = tests.c

CFLAGS = -std=c99 -fsanitize=address -fsanitize=leak -g -Wall -Wextra -Werror

SPINLOCK_DIR = ./spinlock

.PHONY: all spinlock tests

all: spinlock tests

spinlock:
	$(MAKE) -C $(SPINLOCK_DIR) all

tests:
	$(COMPILER) $(CFLAGS) -o tests $(TARGET) -Llib -lspinlock -Wl,-rpath=./lib
	