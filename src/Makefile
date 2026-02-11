CC = cc
ifdef DEBUG
	CFLAGS = -g -Wall -Wextra -std=c11 -D DEBUG
else
	CFLAGS = -g -Wall -Wextra -std=c11
endif
LDFLAGS = -lpthread -lm
includedir = ../include
headers = $(shell find $(includedir) -name "*.h")

JAVA  		:= java
JAVAP 		:= javap
JAVAOBJS 	:= java.o class.o mmr.o util.o instruct.o
JAVAPOBJS 	:= javap.o class.o mmr.o util.o instruct.o
INC 		:= -I$(includedir)

all: ${JAVA} ${JAVAP}

${JAVA}: ${JAVAOBJS}
	$(CC) -o $@ $^ $(LDFLAGS)

${JAVAP}: ${JAVAPOBJS}
	$(CC) -o $@ $^ $(LDFLAGS)

java.o: 	$(includedir)/java.h $(includedir)/class.h $(includedir)/c.h $(includedir)/mmr.h $(includedir)/util.h $(includedir)/instruct.h
javap.o: 	$(includedir)/javap.h $(includedir)/class.h $(includedir)/c.h $(includedir)/mmr.h $(includedir)/util.h	$(includedir)/instruct.h
class.o: 	$(includedir)/class.h $(includedir)/c.h $(includedir)/mmr.h $(includedir)/instruct.h
instruct.o:	$(includedir)/instruct.h $(includedir)/c.h
mmr.o:		$(includedir)/mmr.h
util.o:		$(includedir)/util.h

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -f ${JAVA} ${JAVAP} ${JAVAOBJS} ${JAVAPOBJS}

.PHONY: all clean
