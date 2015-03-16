CC = gcc

SOURCES = server.c RingManager.c UImanager.c main.c

OBJECTS = server.o RingManager.o UImanager.o main.o

ddt: $(OBJECTS)
	gcc -o ddt $(OBJECTS)

server.o: server.h

RingManager.o: RingManager.h

UImanager.o: UImanager.h

clean:
	rm -f *o core ddt *~

depend::
	makedepend $(SOURCES)
# LOL
