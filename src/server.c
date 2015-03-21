#include "server.h"

#define max(A,B) ((A)>=(B)?(A):(B)) /*I think we don't even use this, we just do it manually in the while loop*/

struct Server{
	int i;
	UDPManager  * udpmanager;
	TCPManager  * tcpmanager;
	RingManager * ringmanager;
	int TCPport, shutdown;
};

int ServerProcArg(Server * server, int argc, char ** argv){
	
	char * ringPort = NULL;
	char * bootIP = NULL;
	char * bootPort = NULL;
	int i, read;

	opterr = 0;
	while ((read = getopt(argc, argv, "t:i:p:")) != -1)
		switch (read){					/*I'm sure you can probably clean up this switch somehow, i'll leave that to you*/
			case 't':
				ringPort = optarg;
				break;
			case 'i':
				bootIP = optarg;
				break;
			case 'p':
				bootPort = optarg;
				break;
			case '?':
				if (optopt == 't')
					fprintf(stderr, "Opcao -%c requer argumento.\n", optopt);
				if (optopt == 'i')
					fprintf(stderr, "Opcao -%c requer argumento.\n", optopt);
				if (optopt == 'p')
					fprintf(stderr, "Opcao -%c requer argumento.\n", optopt);
				else if(isprint(optopt))
					printf("Opcao desconhecida '-%c'. Sintaxe: ddt [-t ringport] [-i bootIP] [-p bootport]", optopt);
				else
					printf("Caracter de opcao desconhecido '\\x%x'.\n", optopt);
				return 0;
			default:
				return 0;
		}
	printf("tvalue = %s, ivalue = %s, pvalue = %s\n", ringPort, bootIP, bootPort);

	for (i = optind; i < argc; i++)
		printf ("Argumento invalido %s\n", argv[i]);
	
	if(ringPort != NULL)	server->TCPport = atoi(ringPort);
	if(bootIP != NULL)		UDPManagerSetIP(server->udpmanager, bootIP);
	if(bootPort != NULL)	UDPManagerSetPort(server->udpmanager, atoi(bootPort));
	
	return 0;	
}

Server * ServerInit(int argc, char ** argv){
	Server * server=(Server*)malloc(sizeof(Server));
	
	server->udpmanager  = UDPManagerInit();
	server->ringmanager = RingManagerInit();
	server->tcpmanager  = TCPManagerInit();
	ServerProcArg(server, argc, argv);
	server->shutdown	= 0;
	
	return server;
}

int ServerStart(Server * server){
	
	int argc;
	char buffer[128];
	char args[8][128];
	int maxfd, counter;
	fd_set rfds;
	int n = 0;
	Request *request = RequestCreate();
	
	RequestReset(request);
	
	TCPManagerCreate(server->tcpmanager, server->TCPport);
	
	int fd = TCPSocketCreate();
	TCPSocketBind(fd, server->TCPport);
	if(TCPSocketListen(fd)==-1)exit(1);
	
	while(!server->shutdown){
		
		FD_ZERO(&rfds); maxfd=0;
		
		FD_SET(fd, &rfds);
		maxfd = (fd > maxfd ? fd : maxfd);
		
		UIManagerArm(&rfds,&maxfd);
		RingManagerArm(server->ringmanager,&rfds,&maxfd);
		TCPManagerArm(server->tcpmanager, &rfds, &maxfd);
		
		counter = select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval*)NULL);
		if(counter<0) exit(1);
		
		if(counter == 0) continue;

		n = RingManagerReq(server->ringmanager, &rfds, buffer);
		ServerProcRingReq(server, buffer, n);
		
		n = TCPManagerReq(server->tcpmanager, &rfds, request);
		ServerProcTCPReq(server, request);
		
		n = UIManagerReq(&rfds, request);
		if(n) ServerProcUIReq(server, request);
		
		if(FD_ISSET(fd, &rfds)){
			int newfd = TCPSocketAccept(fd);
			RingManagerNew(server->ringmanager, newfd, 560560565);
		}
	}
	return 0;
}

int ServerStop(Server * server){
									/*To do: close all active fd's*/
	free(server->udpmanager);
	free(server->tcpmanager);
	free(server->ringmanager);
	return 0;
}

int ServerProcRingReq(Server * server, char * buffer, int n){
	if(n <= 0) return n;

	return 1;
}

int ServerProcTCPReq(Server * server, Request * request){

	return 1;
}

int ServerProcUIReq(Server * server, Request * request){
	int i = 0;
	char * command;
	
	if(RequestGetArgCount(request) <= 0) return 0;
	
	printf("You wrote: ");
	
	for(i = 0; i<RequestGetArgCount(request); i++){
		printf("%s,", RequestGetArg(request, i));
		fflush(stdout);
	}
	printf("\n");
	
														/* Idk if you want to leave the translator here,
														 * we should probably make a seperate module for it,
														 * and here we should leave the interpretation for
														 * CON, QRY, etc, perhaps also in a module, as you love to do.*/
	
	command = RequestGetArg(request,0);
	if(strcmp(command,"join") == 0){		/*#Hashtag #switch #functions goes somewhere around here, instead of all this garbage*/
		if(RequestGetArgCount(request) == 3){
			/*Send UDP BQRY x*/
		}
		else if(RequestGetArgCount(request) != 6) return 0;
		
		RingManagerConnect(server->ringmanager, 			/* It seems really annoying to have to get many things only Request knows to feed it into RingManager
															 *	Perhaps we should make RM know what Request is, to feed it directly in?
															 */
						  atoi(RequestGetArg(request, 2)),
						  atoi(RequestGetArg(request, 3)),
						  RequestGetArg(request, 4),
						  atoi(RequestGetArg(request, 5)));
	}
	else if(strcmp(command,"leave") == 0){
		/* To do: Check if only one in ring, if so,
		 * tell boot server to remove node. Else:
		 * REG x succi to boot server and*/
		RingManagerMsg(server->ringmanager, 1, "CON succi s.IP s.TCP");	/* This is incredibly annoying to do here, perhaps migrate this function into RM, 
																		 * after making RM know what Request is, and it can know what to do with the info inside it
																		 */
		/* Maybe make a specific function to CON and QRY
		 * instead of generalized message function
		 */
		
		
		/*Reset all succi, predi, etc*/
	}
	else if(strcmp(command,"show") == 0) RingManagerStatus(server->ringmanager);
	else if(strcmp(command,"search") == 0){
		if(RequestGetArgCount(request) < 2) return 0;
		int k = atoi(RequestGetArg(request, 1));
		int id = RingManagerId(server->ringmanager);
		
		if(RingManagerCheck(server->ringmanager, k)) printf("%i, ip, port", id); /*Add variables for ip and port eventually*/
		else RingManagerMsg(server->ringmanager, 0, "QRY id k"); /*Add int->string support eventually*/
	}
	else if(strcmp(RequestGetArg(request,0),"exit") == 0) server->shutdown = 1;
	
	return 1;
}
