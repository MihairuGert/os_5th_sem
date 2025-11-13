COMPILER = gcc
TARGET = tests.c

CFLAGS = -fsanitize=address -fsanitize=leak -g -Wall -Wextra -Werror

SPINLOCK_DIR = ./spinlock
MUTEX_DIR = ./mutex

.PHONY: all spinlock mutex tests 2.2

all: spinlock mutex tests 2.2

spinlock:
	$(MAKE) -C $(SPINLOCK_DIR) all

mutex:
	$(MAKE) -C $(MUTEX_DIR) all

tests:
	$(COMPILER) $(CFLAGS) -o tests $(TARGET) -Llib -lspinlock -lmutex -Wl,-rpath=./lib

2.2:
	$(COMPILER) $(CFLAGS) -o 2.2my 2.2d.c 2.2my_mutex.c -Llib -lmutex -Wl,-rpath=./lib
	