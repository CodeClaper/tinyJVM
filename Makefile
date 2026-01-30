CC = cc
CFLAGS = -g -O2 -Wall -W -std=c11
LDFLAGS = -lpthread

JAVA  		:= java
JAVAP 		:= javap
JAVAOBJS 	:= java.o
JAVAPOBJS 	:= javap.o

all: ${JAVA} ${JAVAP}

${JAVA}: ${JAVAOBJS}
	$(CC) -o $@ $^ $(LDFLAGS)

${JAVAP}: ${JAVAPOBJS}
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f ${JAVA} ${JAVAP} ${JAVAOBJS} ${JAVAPOBJS}

.PHONY: all clean
