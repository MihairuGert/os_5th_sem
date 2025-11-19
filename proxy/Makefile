COMPILER = gcc

SRCS = $(shell find ./src -name '*.c')
TARGET = ./out/proxy

CFLAGS = -fsanitize=address -fsanitize=leak -g #-Wall -Wextra -Werror

LIB_FLAGS = -luv -lthread_pool
LIB_PATHS = -Llib

.PHONY: all main clean client $(LIBRARIES)

all: $(LIBRARIES) main

$(LIBRARIES):
	$(MAKE) -C ./lib/$@ all

main: $(LIBRARIES)
	$(COMPILER) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIB_PATHS) $(LIB_FLAGS) -Wl,-rpath=./lib

client:
	$(COMPILER) client.c -o ./out/test_client

clean:
	$(foreach lib, $(LIBRARIES), $(MAKE) -C ./lib/$(lib) clean;)
	rm -f $(TARGET)