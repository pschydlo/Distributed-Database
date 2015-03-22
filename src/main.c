#include <stdlib.h>
#include "server.h"

int main(int argc, char **argv){
	Server * server;
	
	server = ServerInit(argc, argv); /*Maybe process args in main, but looking good for now*/
	ServerStart(server);
	ServerStop(server);
	
	exit(0);
}

