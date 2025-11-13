TARGET_1 = queue-example
SRCS_1 = queue.c queue-example.c

TARGET_2 = queue-threads
SRCS_2 = queue.c queue-threads.c

TARGET_3 = 2.2a-c
SRCS_3 = 2.2a-c.c queue-threads.c 

TARGET_4 = 2.1b
SRCS_4 = 2.1b.c queue.c 

TARGET_5 = 2.2cqueue-threads
SRCS_5 = 2.2a-c.c queue-threads.c

TARGET_6 = 2.2d-threads
SRCS_6 = 2.2a-c.c 2.2d.c

TARGET_7 = 2.2e-threads
SRCS_7 = 2.2e.c 2.2d.c

TARGET_8 = 2.2f-threads
SRCS_8 = 2.2f.c 2.2d.c

TARGET_9 = 2.2g-threads
SRCS_9 = 2.2g.c 2.2d.c

TARGET_10 = 2.2my
SRCS_10 = 2.2my_mutex.c 2.2d.c

CC=gcc
RM=rm
CFLAGS= -g -Wall -fsanitize=address -static-libasan
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET_1} ${TARGET_2} ${TARGET_3} ${TARGET_4} $(TARGET_5) $(TARGET_6) $(TARGET_7) $(TARGET_8) $(TARGET_9) $(TARGET_10)

${TARGET_1}: queue.h ${SRCS_1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_1} ${LIBS} -o ${TARGET_1}

${TARGET_2}: queue.h ${SRCS_2}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_2} ${LIBS} -o ${TARGET_2}

${TARGET_3}: queue.h ${SRCS_3}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_3} ${LIBS} -o ${TARGET_3}

${TARGET_4}: queue.h ${SRCS_4}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_4} ${LIBS} -o ${TARGET_4}

${TARGET_5}: queue.h ${SRCS_5}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_5} ${LIBS} -o ${TARGET_5}

${TARGET_6}: queue.h ${SRCS_6}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_6} ${LIBS} -o ${TARGET_6}

${TARGET_7}: queue.h ${SRCS_7}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_7} ${LIBS} -o ${TARGET_7}

${TARGET_8}: queue.h ${SRCS_8}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_8} ${LIBS} -o ${TARGET_8}

${TARGET_9}: queue.h ${SRCS_9}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_9} ${LIBS} -o ${TARGET_9}

${TARGET_10}: queue.h ${SRCS_10}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_10} ${LIBS} -o ${TARGET_10}

clean:
	${RM} -f *.o ${TARGET_1} ${TARGET_2}
