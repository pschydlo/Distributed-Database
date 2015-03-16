CC = gcc

SOURCES = TCPSocket.c RingManager.c UImanager.c server.c main.c 

OBJECTS = TCPSocket.o RingManager.o UImanager.o server.o main.o

ddt: $(OBJECTS)
	gcc -o ddt $(OBJECTS)

server.o: server.h

RingManager.o: RingManager.h

UImanager.o: UImanager.h

TCPSocket.o: TCPSocket.h

clean:
	rm -f *o core ddt *~

depend:
	makedepend $(SOURCES)
# LOL
