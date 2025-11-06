COMPILER = gcc
TARGET = tests.c

CFLAGS = -fsanitize=address -fsanitize=leak -g -Wall -Wextra -Werror

SPINLOCK_DIR = ./spinlock
MUTEX_DIR = ./mutex

.PHONY: all spinlock mutex tests

all: spinlock mutex tests

spinlock:
	$(MAKE) -C $(SPINLOCK_DIR) all

mutex:
	$(MAKE) -C $(MUTEX_DIR) all

tests:
	$(COMPILER) $(CFLAGS) -o tests $(TARGET) -Llib -lspinlock -lmutex -Wl,-rpath=./lib
	