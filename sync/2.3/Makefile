TARGET_1 = mutex
SRCS_1 = 2.3m.c

CC=gcc
RM=rm
CFLAGS= -g -fsanitize=address -fsanitize=leak -Wall -Wextra -Werror
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET_1}

${TARGET_1}: ${SRCS_1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_1} ${LIBS} -o ${TARGET_1}

clean:
	${RM} -f *.o ${TARGET_1}
