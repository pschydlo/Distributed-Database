CC = gcc

SOURCES = server.c TCPmanager.c UImanager.c main.c

OBJECTS = server.o TCPmanager.o UImanager.o main.o

ddt: $(OBJECTS)
	gcc -g -o ddt $(OBJECTS)

server.o: server.h

TCPmanager.o: TCPmanager.h

UImanager.o: UImanager.h

clean:
	rm -f *o core ddt *~

depend::
	makedepend $(SOURCES)
# LOL
