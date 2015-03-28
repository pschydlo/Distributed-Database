CC=gcc
CFLAGS=-g -Wall

DEPS = utilities.h Request.h RoutingTable.h UDPSocket.h TCPSocket.h TCPManager.h UDPManager.h HTTPManager.h RingManager.h UIManager.h server.h main.h 

OBJ = utilities.o Request.o RoutingTable.o UDPSocket.o TCPSocket.o TCPManager.o UDPManager.o HTTPManager.c RingManager.o UIManager.o server.o main.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ddt: $(OBJ)
	gcc -o ddt $^ $(CFLAGS)

clean:
	rm -f *o core ddt *~
