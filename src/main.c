#include <stdlib.h>
#include "server.h"

int main(int argc, char **argv){
	
	Server * server;
	
	server = ServerInit();
	
	ServerConfigPort(server, 58002);
	if (argc > 1) ServerConfigPort(server, atoi(argv[1]));
	
	ServerStart(server);
	ServerStop(server);
	
	exit(0);
}
