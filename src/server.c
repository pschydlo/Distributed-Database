#include "server.h"

#define max(A,B) ((A)>=(B)?(A):(B))

struct Server{
	int i;
	RingManager * ringmanager;
};

Server * ServerInit(){
	Server * server=(Server*)malloc(sizeof(Server));
	server->ringmanager=RingManagerInit();
	return server;
}

int ServerStart(Server * server){
	
	int shutdown = 0;
	char buffer[128];
	int maxfd, counter;
	fd_set rfds;
	
	while(!shutdown){
		
		FD_ZERO(&rfds); maxfd=0;
		
		UImanagerArm(&rfds,&maxfd);	
		RingManagerArm(server->ringmanager,&rfds,&maxfd);
		
		counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval*)NULL);
			if(counter<0)exit(1);
		
		puts("Herpderp");
		fflush(stdout);
		
		if(counter==0)continue;

		RingManagerReq(server->ringmanager,&rfds,buffer);
		
		UImanagerReq(&rfds,buffer);

	}
}

int ServerStop(Server * server){
	free(server);
	return 0;
}
