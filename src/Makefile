CC = cc
CFLAGS = -g -O2 -Wall -Wextra -std=c11
LDFLAGS = -lpthread -lm
includedir = ../include
headers = $(shell find $(includedir) -name "*.h")

JAVA  		:= java
JAVAP 		:= javap
JAVAOBJS 	:= java.o class.o mmr.o util.o
JAVAPOBJS 	:= javap.o class.o mmr.o util.o
INC 		:= -I$(includedir)

all: ${JAVA} ${JAVAP}

${JAVA}: ${JAVAOBJS}
	$(CC) -o $@ $^ $(LDFLAGS)

${JAVAP}: ${JAVAPOBJS}
	$(CC) -o $@ $^ $(LDFLAGS)

java.o: 	$(includedir)/java.h $(includedir)/class.h $(includedir)/c.h $(includedir)/mmr.h $(includedir)/util.h
javap.o: 	$(includedir)/javap.h $(includedir)/class.h $(includedir)/c.h $(includedir)/mmr.h $(includedir)/util.h
class.o: 	$(includedir)/class.h $(includedir)/c.h $(includedir)/mmr.h
mmr.o:		$(includedir)/mmr.h
util.o:		$(includedir)/util.h

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -f ${JAVA} ${JAVAP} ${JAVAOBJS} ${JAVAPOBJS}

.PHONY: all clean
