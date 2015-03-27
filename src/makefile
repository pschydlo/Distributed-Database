CC=gcc
CFLAGS=-g -Wall
DEPS = Request.h UDPSocket.h TCPSocket.h TCPManager.h UDPManager.h RingManager.h UIManager.h server.h main.h 

OBJ = Request.o UDPSocket.o TCPSocket.o TCPManager.o UDPManager.o RingManager.o UIManager.o server.o main.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ddt: $(OBJ)
	gcc -o ddt $^ $(CFLAGS)

clean:
	rm -f *o core ddt *~
