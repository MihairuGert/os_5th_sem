TARGET_1 = mutex
SRCS_1 = 2.3m.c

TARGET_2 = mymutex
SRCS_2 = 2.3-2.4.c

CC=gcc
RM=rm
CFLAGS= -g -fsanitize=address -fsanitize=leak -Wall -Wextra -Werror
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET_1} ${TARGET_2}

${TARGET_2}: ${SRCS_2}
	${CC} ${CFLAGS} ${SRCS_2} -o ${TARGET_2} -L./ -lspinlock -lmutex -Wl,-rpath=.

${TARGET_1}: ${SRCS_1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_1} ${LIBS} -o ${TARGET_1}

clean:
	${RM} -f *.o ${TARGET_1}
	${RM} -f *.o ${TARGET_2}
