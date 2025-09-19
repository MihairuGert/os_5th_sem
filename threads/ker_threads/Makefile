TARGET = test.c
LIB_NAME = mythread

CFLAGS = -std=c99

all: 
	gcc $(CFLAGS) -shared -fPIC -o lib$(LIB_NAME).so mythread.c;
	gcc $(CFLAGS) -o out $(TARGET) -L. -l$(LIB_NAME) -Wl,-rpath=.