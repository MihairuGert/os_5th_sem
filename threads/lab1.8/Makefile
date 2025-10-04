COMPILER = gcc
TARGET = cluster.c

THR_PL_LIB_NAME = thpl
THR_PL_SOURCE = thread_pool.c

CFLAGS = -std=c99 -fsanitize=address -g

all:
	$(COMPILER) $(CFLAGS) -shared -fPIC -o lib$(THR_PL_LIB_NAME).so $(THR_PL_SOURCE);
	$(COMPILER) $(CFLAGS) -o out $(TARGET) -L. -l$(THR_PL_LIB_NAME) -Wl,-rpath=.

