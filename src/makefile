CC = gcc

SOURCES = Request.c TCPSocket.c TCPManager.c RingManager.c UImanager.c server.c main.c 

OBJECTS = Request.o TCPSocket.o TCPManager.o RingManager.o UImanager.o server.o main.o

ddt: $(OBJECTS)
	gcc -o ddt $(OBJECTS)

server.o: server.h

RingManager.o: RingManager.h

UImanager.o: UImanager.h

TCPManager.o: TCPManager.h

TCPSocket.o: TCPSocket.h

Request.o: Request.h

clean:
	rm -f *o core ddt *~

depend:
	makedepend $(SOURCES)
# LOL
