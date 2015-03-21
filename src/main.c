#include <stdlib.h>
#include "server.h"

int main(int argc, char **argv){
	Server * server;
	
	server = ServerInit(argc, argv);
	ServerStart(server);
	ServerStop(server);
	
	exit(0);
}
