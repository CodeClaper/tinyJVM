CC = cc
CFLAGS = -g -O2 -Wall -Wextra -std=c11
LDFLAGS = -lpthread -lm

JAVA  		:= java
JAVAP 		:= javap
JAVAOBJS 	:= java.o class.o
JAVAPOBJS 	:= javap.o class.o

all: ${JAVA} ${JAVAP}

${JAVA}: ${JAVAOBJS}
	$(CC) -o $@ $^ $(LDFLAGS)

${JAVAP}: ${JAVAPOBJS}
	$(CC) -o $@ $^ $(LDFLAGS)

java.o: java.h class.h
javap.o: javap.h class.h

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f ${JAVA} ${JAVAP} ${JAVAOBJS} ${JAVAPOBJS}

.PHONY: all clean
