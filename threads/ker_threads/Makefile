TARGET = test.c
LIB_NAME = mythread

all: 
	gcc -shared -fPIC -o lib$(LIB_NAME).so mythread.c;
	gcc -o out $(TARGET) -L. -l$(LIB_NAME) -Wl,-rpath=.