#include "server.h"

#define max(A,B) ((A)>=(B)?(A):(B)) /*I think we don't even use this, we just do it manually in the while loop*/

struct Server{
	int isBoot;
	UDPManager  * udpmanager;
	TCPManager  * tcpmanager;
	RingManager * ringmanager;
	int TCPport, shutdown;
};

int ServerProcArg(Server * server, int argc, char ** argv){
	
	char * ringPort = NULL;
	char * bootIP = NULL;
	char * bootPort = NULL;
	int i, opt;

	opterr = 0;
	while ((opt = getopt(argc, argv, "t:i:p:")) != -1){
		switch (opt){					/*I'm sure you can probably clean up this switch somehow, i'll leave that to you*/
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
					fprintf(stderr, "Opcao desconhecida '-%c'. Sintaxe: ddt [-t ringport] [-i bootIP] [-p bootport]\n", optopt);
				else
					fprintf(stderr, "Caracter de opcao desconhecido '\\x%x'.\n", optopt);
				return 0;
			default:
				return 0;
		}
  }
  
	printf("tvalue = %s, ivalue = %s, pvalue = %s\n", ringPort, bootIP, bootPort);

	for (i = optind; i < argc; i++)
		fprintf (stderr, "Argumento invalido %s\n", argv[i]);
	
	if(ringPort != NULL)	server->TCPport = atoi(ringPort);
	if(bootIP   != NULL)	UDPManagerSetIP(server->udpmanager, bootIP);
	if(bootPort != NULL)	UDPManagerSetPort(server->udpmanager, atoi(bootPort));

	return 0;	
}

Server * ServerInit(int argc, char ** argv){
	Server * server=(Server*)malloc(sizeof(Server));
	
	server->isBoot		= 0;
	server->shutdown	= 0;
	server->udpmanager	= UDPManagerInit();
	server->tcpmanager	= TCPManagerInit();
	
	ServerProcArg(server, argc, argv);
  
  server->ringmanager	= RingManagerInit(server->TCPport);
	
	return server;
}

int ServerStart(Server * server){
	
	int maxfd, counter;
	fd_set rfds;
	int n = 0;
  
	Request *request = RequestCreate();
	RequestReset(request);
	
	TCPManagerStart(server->tcpmanager, server->TCPport);
	UDPManagerCreate(server->udpmanager);
	
	while(!(server->shutdown)){
		
		FD_ZERO(&rfds); maxfd=0;
		
		UIManagerArm(&rfds,&maxfd);
		RingManagerArm(server->ringmanager,&rfds,&maxfd);
		TCPManagerArm(server->tcpmanager, &rfds, &maxfd);
		
		counter = select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval*)NULL);
		
    if(counter<0) exit(1);
		if(counter == 0) continue;

    RequestReset(request);
		n = RingManagerReq(server->ringmanager, &rfds, request);
		if(n) ServerProcRingReq(server, request);
		
    RequestReset(request);
		n = TCPManagerReq(server->tcpmanager, &rfds, request);
		if(n) ServerProcTCPReq(server, request);
		
    RequestReset(request);
		n = UIManagerReq(&rfds, request);
		if(n) ServerProcUIReq(server, request);
	}
  
  RequestDestroy(request);
  
	return 0;
}

int ServerStop(Server * server){
  /*To do: close all active fd's*/
	
  UDPManagerStop(server->udpmanager);
  free(server->udpmanager);	/*Haven't freed addr yet*/
	free(server->tcpmanager);
	free(server->ringmanager);
  
  /*free(request);*/
	
  return 0;
}

int ServerProcRingReq(Server * server, Request * request){
  int i = 0;
  
  if(RequestGetArgCount(request) <= 0) return 0;
  
  printf("Ring wrote: ");
  
  for(i = 0; i<RequestGetArgCount(request); i++){
    printf("%s,", RequestGetArg(request, i));
    fflush(stdout);
  }
  
  printf("\n");
  
  if(strcmp(RequestGetArg(request,0),"RSP") == 0) {
    if(RequestGetArgCount(request) != 6) return 0; 
    
    int originID = atoi(RequestGetArg(request, 1));
    int searchID = atoi(RequestGetArg(request, 2));
    
    //if(originID ==  RingManagerId(server->ringmanager)){
      //printf("This one is ours!");
      //fflush(stdout);
    //}else{
      RingManagerRsp(server->ringmanager, originID, searchID, 2, RequestGetArg(request, 3), 9000);
    //}        
  }
  
  if(strcmp(RequestGetArg(request,0),"CON") == 0){
    //if(RequestGetArgCount(request) != 4) return 0; /*comented for testing purposes!*/
    
    int destinationPort  = atoi(RequestGetArg(request, 3));
    char * destinationIP = RequestGetArg(request, 2);
    int destinationID    = atoi(RequestGetArg(request, 1));
    
    RingManagerConnect(server->ringmanager, 1, destinationID, destinationIP, destinationPort);
    TCPManagerRemoveSocket(server->tcpmanager, RequestGetFD(request));
  }
  
	return 1;
}

int ServerProcTCPReq(Server * server, Request * request){

  int i = 0;
  
  if(RequestGetArgCount(request) <= 0) return 0;
  
  printf("External wrote: ");
  
  for(i = 0; i<RequestGetArgCount(request); i++){
    printf("%s,", RequestGetArg(request, i));
    fflush(stdout);
  }
  
  printf("\n");
  
  if(strcmp(RequestGetArg(request,0),"NEW") == 0){
    //if(RequestGetArgCount(request) != 4) return 0; /*comented for testing purposes!*/
    
    int originPort  = atoi(RequestGetArg(request, 3));
    char * originIP = RequestGetArg(request, 2);
    int originID    = atoi(RequestGetArg(request, 1));
    
    RingManagerNew(server->ringmanager, RequestGetFD(request), originID, originIP, originPort);
    TCPManagerRemoveSocket(server->tcpmanager, RequestGetFD(request));
  }
  
  if(strcmp(RequestGetArg(request,0),"CON") == 0){
    //if(RequestGetArgCount(request) != 4) return 0; /*comented for testing purposes!*/
    
    int destinationPort  = atoi(RequestGetArg(request, 3));
    char * destinationIP = RequestGetArg(request, 2);
    int destinationID    = atoi(RequestGetArg(request, 1));
    
    RingManagerConnect(server->ringmanager, 1, destinationID, destinationIP, destinationPort);
    TCPManagerRemoveSocket(server->tcpmanager, RequestGetFD(request));
  }
  
  return 1;
}

int ServerProcUIReq(Server * server, Request * request){
	int i = 0;
	char * command;
	
	if(RequestGetArgCount(request) <= 0) return 0;
	
	printf("You wrote: ");
	
	for(i = 0; i<RequestGetArgCount(request); i++){
		printf("%s ", RequestGetArg(request, i));
		fflush(stdout);
	}
  
	printf("\n");
	
  /* Idk if you want to leave the translator here,
   * we should probably make a seperate module for it,
   * and here we should leave the interpretation for
   * CON, QRY, etc, perhaps also in a module, as you love to do.*/

	command = RequestGetArg(request,0);
	int count = RequestGetArgCount(request);/*Perhaps change this*/
	if(strcmp(command,"join") == 0){		/*#Hashtag #switch #functions goes somewhere around here, instead of all this garbage*/
		if(count != 6 && count != 3) return 0;
		if(count == 3){
			/*Send UDP BQRY x*/
			/*if(BQRY == EMPTY)*/
			UDPManagerJoin(server->udpmanager, 9);
			/*else send(ID)*/
			/*receive "SUCC" from succi*/
		}		
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
  else if(strcmp(command,"rsp") == 0) RingManagerRsp(server->ringmanager, 1, 1, 1, "127.0.0.1", 9000);
	else if(strcmp(command,"search") == 0){		/*Reminder: limit commands if user is not connect to ring*/
		if(count < 2) return 0;
    
		int search = atoi(RequestGetArg(request, 1));
		int id     = RingManagerId(server->ringmanager);
		
		if(RingManagerCheck(server->ringmanager, search)) printf("%i, ip, port", id); /*Add variables for ip and port eventually*/
		else RingManagerQuery(server->ringmanager, id, search); /*Add int->string support eventually*/
	}
	else if(strcmp(RequestGetArg(request,0),"boops") == 0) RingManagerMsg(server->ringmanager, 0, "Boop\n");/*Debugging boops*/
	else if(strcmp(RequestGetArg(request,0),"boopp") == 0) RingManagerMsg(server->ringmanager, 1, "Boop\n");
	else if(strcmp(RequestGetArg(request,0),"exit") == 0) server->shutdown = 1;
	else printf("Comando não reconhecido\n");
	
	return 1;
}
