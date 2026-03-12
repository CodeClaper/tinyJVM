CC = cc
ifdef DEBUG
	CFLAGS = -g -Wall -Wextra -gdwarf-2 -std=c11 -D DEBUG
else
	CFLAGS = -O2 -Wall -std=c11
endif
LDFLAGS = -lpthread -lm
includedir = ../include
headers = $(shell find $(includedir) -name "*.h")

JAVA  		:= java
JAVAP 		:= javap
JAVAOBJS 	:= java.o class.o mmr.o util.o instruct.o frame.o heap.o method.o native.o
JAVAPOBJS 	:= javap.o class.o mmr.o util.o instruct.o frame.o heap.o method.o native.o
INC 		:= -I$(includedir)

all: ${JAVA} ${JAVAP}

${JAVA}: ${JAVAOBJS}
	$(CC) -o $@ $^ $(LDFLAGS)

${JAVAP}: ${JAVAPOBJS}
	$(CC) -o $@ $^ $(LDFLAGS)

java.o: 	$(includedir)/java.h $(includedir)/class.h $(includedir)/c.h $(includedir)/mmr.h $(includedir)/util.h $(includedir)/instruct.h $(includedir)/frame.h $(includedir)/heap.h $(includedir)/data.h $(includedir)/native.h $(includedir)/method.h
javap.o: 	$(includedir)/javap.h $(includedir)/class.h $(includedir)/c.h $(includedir)/mmr.h $(includedir)/util.h $(includedir)/instruct.h $(includedir)/heap.h
class.o: 	$(includedir)/class.h $(includedir)/c.h $(includedir)/mmr.h $(includedir)/instruct.h
instruct.o:	$(includedir)/instruct.h $(includedir)/c.h $(includedir)/frame.h
frame.o:	$(includedir)/frame.h $(includedir)/c.h $(includedir)/java.h $(includedir)/mmr.h $(includedir)/util.h
heap.o:		$(includedir)/heap.h $(includedir)/c.h $(includedir)/mmr.h $(includedir)/util.h
native.o:	$(includedir)/native.h $(includedir)/c.h
method.o:	$(includedir)/method.h $(includedir)/data.h
mmr.o:		$(includedir)/mmr.h
util.o:		$(includedir)/util.h

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -f ${JAVA} ${JAVAP} ${JAVAOBJS} ${JAVAPOBJS}

.PHONY: all clean
