COMPILER = gcc

SRCS = $(shell find ./src -name '*.c')
TARGET = ./out/proxy

CFLAGS = -O0 -DDEBUG -fsanitize=address -fsanitize=leak -g -fno-omit-frame-pointer #-Wall -Wextra -Werror

LIB_FLAGS = -l:libuv.a -lthread_pool -lpicohttpparser -lcurl
LIB_PATHS = -Llib

LIBRARIES = picohttpparser

.PHONY: all main clean client $(LIBRARIES) run fmt

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

run:
	set UV_THREADPOOL_SIZE = 10
	sudo ./out/proxy

fmt:
	find . -name "*.c" -o -name "*.h" | xargs clang-format -i -style=WebKit