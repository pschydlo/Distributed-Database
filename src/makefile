CC = gcc

SOURCES = Request.c TCPSocket.c TCPManager.c UDPManager.c RingManager.c UIManager.c server.c main.c 

OBJECTS = Request.o TCPSocket.o TCPManager.o UDPManager.o RingManager.o UIManager.o server.o main.o

ddt: $(OBJECTS)
	gcc -o ddt $(OBJECTS)

server.o: server.h

RingManager.o: RingManager.h

UIManager.o: UIManager.h

UDPManager.o: UDPManager.h

TCPManager.o: TCPManager.h

TCPSocket.o: TCPSocket.h

Request.o: Request.h

clean:
	rm -f *o core ddt *~

depend:
	makedepend $(SOURCES)
# LOL
