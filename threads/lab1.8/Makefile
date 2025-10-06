COMPILER = gcc
TARGET = cluster.c

CFLAGS = -std=c99 -fsanitize=address -g

COROLIB_DIR = ./coroutines
THREAD_POOL_DIR = ./thread_pool 

.PHONY: all corolib thread_pool cluster clean

all: corolib thread_pool cluster

corolib:
	$(MAKE) -C $(COROLIB_DIR) all

thread_pool:
	$(MAKE) -C $(THREAD_POOL_DIR) all

cluster:
	$(COMPILER) $(CFLAGS) -o out $(TARGET) -Llib -lthread_pool -lcoro -Wl,-rpath=./lib

clean:
	$(MAKE) -C $(COROLIB_DIR) clean
	$(MAKE) -C $(THREAD_POOL_DIR) clean
	rm -f out lib$(THR_PL_LIB_NAME).so
	