#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int TCPSocketCreate();
int TCPSocketConnect(int fd, char * ip, int port);

int TCPSocketBind(int fd, int port);
int TCPSocketListen(int fd);
int TCPSocketAccept(int fd);

int TCPSocketWrite(int fd, char * buffer, int nbytes);