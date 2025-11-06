COMPILER = gcc

SRCS = $(shell find ./src -name '*.c')
TARGET = ./out/proxy

CFLAGS = -fsanitize=address -fsanitize=leak -g -Wall -Wextra -Werror

LIBRARIES = thread_pool hashtable
LIB_DIRS = $(addprefix ./lib/, $(LIBRARIES))

LIB_FLAGS = $(addprefix -l, $(LIBRARIES))
LIB_PATHS = -Llib

.PHONY: all main clean $(LIBRARIES)

all: $(LIBRARIES) main

$(LIBRARIES):
	$(MAKE) -C ./lib/$@ all

main: $(LIBRARIES)
	$(COMPILER) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIB_PATHS) $(LIB_FLAGS) -Wl,-rpath=./lib

clean:
	$(foreach lib, $(LIBRARIES), $(MAKE) -C ./lib/$(lib) clean;)
	rm -f $(TARGET)