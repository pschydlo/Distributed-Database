#include "server.h"

#define max(A,B) ((A)>=(B)?(A):(B)) /*I think we don't even use this, we just do it manually in the while loop*/

#define JOIN 	592
#define LEAVE   1253
#define SHOW 	657
#define RSP 	350
#define SEARCH 2654
#define BOOPP  1140
#define BOOPS  1143
#define STREAM 2955
#define CLOSESTREAM 69707
#define HASH   586
#define SEND   692
#define EXIT   622
#define CHECK  1097

struct Server{
	int isBoot;
	UDPManager  * udpmanager;
	TCPManager  * tcpmanager;
	RingManager * ringmanager;
	char ip[16];
	int TCPport, shutdown;
};

int hash(char * input);

int ServerProcArg(Server * server, int argc, char ** argv){
	
	char * ringPort = NULL;
	char * bootIP 	= NULL;
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

Server * ServerInit(int argc, char ** argv, char * ip){
	Server * server=(Server*)malloc(sizeof(Server));
	
	server->isBoot		= 0;
	server->shutdown	= 0;
	server->TCPport 	= 9000;
	server->udpmanager	= UDPManagerInit();
	server->tcpmanager	= TCPManagerInit();
	
	ServerProcArg(server, argc, argv);
  
	strcpy(server->ip, ip);
	server->ringmanager	= RingManagerInit(server->ip, server->TCPport);
	
	
	printf("%s\n", ip);
	fflush(stdout);
	
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
		UDPManagerArm(server->udpmanager, &rfds, &maxfd);
		
		counter = select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval*)NULL);
		
		if(counter<0) exit(1);
		if(counter == 0) continue;

		do{
			n = 0;
			
			RequestReset(request);
			if(RingManagerReq(server->ringmanager, &rfds, request)){
				ServerProcRingReq(server, request);
				n++;
			}

			RequestReset(request);
			if(TCPManagerReq(server->tcpmanager, &rfds, request)){
				ServerProcTCPReq(server, request);
				n++;
			}; 

			RequestReset(request);
			if(UDPManagerReq(server->udpmanager, &rfds, request)){
				ServerProcUDPReq(server, request);
				n++;
			}
			
			RequestReset(request);
			if(UIManagerReq(&rfds, request)){
				ServerProcUIReq(server, request);
				n++;
			}
		} while(n != 0);
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

int ServerProcUDPReq(Server * server, Request * request){
	
	if(RequestGetArgCount(request) <= 0) return 0;
	if(strcmp(RequestGetArg(request, 0), "NOK") == 0) printf("Problem with UDP\n");
	if(strcmp(RequestGetArg(request, 0), "EMPTY") == 0) UDPManagerReg(server->udpmanager, server->ip, server->TCPport);
	if(strcmp(RequestGetArg(request, 0), "BRSP") == 0){
		/*Stop freaking out, I left this unmodularized on purpose so I wouldn't put it in the wrong place
		 * We can decide to place this entire section in the correct place eventually,
		 * also, this is pretty much just a test*/
		int destID 		= atoi(RequestGetArg(request, 2));
		char * destIP 	= RequestGetArg(request, 3);
		int destPort 	= atoi(RequestGetArg(request, 4));
		
		int id = UDPManagerID(server->udpmanager);
		
		if(id != destID){
		
		int n, idfd = TCPSocketCreate();
		if((n = TCPSocketConnect(idfd, destIP, destPort)) < 0){
			printf("IP: %s, Port: %d", destIP, destPort);
			printf("Could not connect to boot vertex.");
			exit(1);
		} /*ERROR! checking to be done*/
		
		char msg[50];
		sprintf(msg, "ID %d\n", UDPManagerID(server->udpmanager));
  
		write(idfd, msg, strlen(msg));
		return 0;
	/*Until here*/
        }else printf("ID %d occupied, please choose another\n", destID);
	}
		
	int i;
	for(i = 0; i<RequestGetArgCount(request); i++){
		printf("%s,", RequestGetArg(request, i));
		fflush(stdout);
	}
	printf("\n");
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
    char * responsibleIP = RequestGetArg(request, 3);
    int responsiblePort = atoi(RequestGetArg(request, 4));
    
    if(originID == RingManagerId(server->ringmanager)){
      //Handle response 
    }else{
      RingManagerRsp(server->ringmanager, originID, searchID, 2, responsibleIP, responsiblePort);
    }
    
    
    //if(originID ==  RingManagerId(server->ringmanager)){
      //printf("This one is ours!");
      //fflush(stdout);
    //}else{
      //RingManagerRsp(server->ringmanager, originID, searchID, 2, RequestGetArg(request, 3), 9000);
    //}        
  }
  
  if(strcmp(RequestGetArg(request,0),"CON") == 0){
    if(RequestGetArgCount(request) != 4) return 0; /*comented for testing purposes!*/
    
    int id   = RingManagerId(server->ringmanager);
    int succiID    = atoi(RequestGetArg(request, 1));
    char * succiIP = RequestGetArg(request, 2); 
    int succiPort = atoi(RequestGetArg(request, 3)); 
                    
    
		RingManagerConnect(server->ringmanager, 1, id, succiID, succiIP, succiPort);
  }
  
  if(strcmp(RequestGetArg(request,0),"QRY") == 0) {
    if(RequestGetArgCount(request) != 3) return 0; 
    
    int originID = atoi(RequestGetArg(request, 1));
    int searchID = atoi(RequestGetArg(request, 2));
    
    if(RingManagerCheck(server->ringmanager, searchID)){
      RingManagerRsp(server->ringmanager, originID, searchID, RingManagerId(server->ringmanager), server->ip, server->TCPport);
    }else{
      RingManagerQuery(server->ringmanager, originID, searchID );
    }
    
    
    //RingManagerCheck(server->ringmanager, searchID)){
      //printf("This one is ours!");
      //fflush(stdout);
    //}else{
    //} 
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
    
    int originID    = atoi(RequestGetArg(request, 1));
    char * originIP = RequestGetArg(request, 2);
    int originPort  = atoi(RequestGetArg(request, 3));
    
    RingManagerNew(server->ringmanager, RequestGetFD(request), originID, originIP, originPort);
    TCPManagerRemoveSocket(server->tcpmanager, RequestGetFD(request));
  }
  
  if(strcmp(RequestGetArg(request,0),"CON") == 0){
    //if(RequestGetArgCount(request) != 4) return 0; /*comented for testing purposes!*/
    
    int id   = RingManagerId(server->ringmanager);
    int succiID    = atoi(RequestGetArg(request, 1));
    char * succiIP = RequestGetArg(request, 2); 
    int succiPort = atoi(RequestGetArg(request, 3)); 
                    
    
		RingManagerConnect(server->ringmanager, 1, id, succiID, succiIP, succiPort);
  }
  /*A esta hora da noite só escrevo porcaria. Isto ainda falta muito pensar*/
  /*if(strcmp(RequestGetArg(request,0),"ID") == 0){
    if(RequestGetArgCount(request) != 2) return 0;
    
		int search = atoi(RequestGetArg(request, 1));
		int id     = RingManagerId(server->ringmanager);
		
		if(RingManagerCheck(server->ringmanager, search)){
            char msg[50];
            sprintf(msg, "SUCC %d %s %d\n", UDPManagerID(server->udpmanager), server->ip, server->TCPport);
            write(TCPManagerIDfd(server->tcpmanager), msg, strlen(msg));
            }
		else RingManagerQuery(server->ringmanager, id, search);
      
  }*/
  
  if(strcmp(RequestGetArg(request, 0), "SUCC") == 0){
    int destID = atoi(RequestGetArg(request, 1));
    if(UDPManagerID(server->udpmanager) == destID){
        printf("ID %d already in use in ring, please select another\n", destID);
        return 0;
    }
        char * destIP   = RequestGetArg(request, 2); 
        int destPort    = atoi(RequestGetArg(request, 3)); 

        RingManagerConnect(server->ringmanager,
                        UDPManagerRing(server->udpmanager), 
                        UDPManagerID(server->udpmanager), 
                        destID, destIP, destPort);
    
    }
  
  return 1;
}

int ServerProcUIReq(Server * server, Request * request){
	
	int argCount = RequestGetArgCount(request);
	if(argCount <= 0) return 0;

	char * command = RequestGetArg(request,0);
	int code = hash(command);
	
	switch(code){
		case(JOIN):
		{
			if(argCount == 3){
				/*Send UDP BQRY x*/
				/*if(BQRY == EMPTY)*/
				/*RingManagerSetId(server->ringmanager, atoi(RequestGetArg(request, 2)));*/
				UDPManagerJoin(server->udpmanager, atoi(RequestGetArg(request, 1)), atoi(RequestGetArg(request, 2)));
				/*UDPManagerReg(server->udpmanager, atoi(RequestGetArg(request, 1)), atoi(RequestGetArg(request, 2)));*/
				/*else send(ID)*/
				/*receive "SUCC" from succi*/
			}else if(argCount == 6){
			
				int ring = atoi(RequestGetArg(request, 1));
				int id   = atoi(RequestGetArg(request, 2));
				int succiID    = atoi(RequestGetArg(request, 3));
				char * succiIP = RequestGetArg(request, 4); 
				int succiPort  = atoi(RequestGetArg(request, 5)); 

				RingManagerConnect(server->ringmanager, ring, id, succiID, succiIP, succiPort);
			}
			break;
		}
		case(LEAVE):
			/* To do: Check if only one in ring, if so,
			 * tell boot server to remove node. Else:
			 * REG x succi to boot server and*/
			RingManagerMsg(server->ringmanager, 1, "CON succi s.IP s.TCP");	
			break;	
		case(SHOW):
			RingManagerStatus(server->ringmanager);
			break;
		case(RSP):
			RingManagerRsp(server->ringmanager, 0, 1, RingManagerId(server->ringmanager), server->ip, server->TCPport);
			break;	
		case (SEARCH):
		{
			/*Reminder: limit commands if user is not connect to ring*/
			if(RequestGetArgCount(request) < 2) return 0;

			int search = atoi(RequestGetArg(request, 1));
			int id     = RingManagerId(server->ringmanager);

			if( RingManagerCheck(server->ringmanager, search) ){
				printf("Yey, don't have to go far: %i, ip, port\n", id); /*Add variables for ip and port eventually*/
			} else {
				RingManagerQuery(server->ringmanager, id, search); /*Add int->string support eventually*/
			}
			break;
		}
		case(BOOPP):
			RingManagerMsg(server->ringmanager, 1, "Boop\nBoop\n");
			
			break;
		case(BOOPS):
			RingManagerMsg(server->ringmanager, 0, "Boop\nBoop\n");
			break;
		case(STREAM):
		{
			char buffer[100];
			fgets(buffer, 100, stdin);
			buffer[strlen(buffer)-1] = '\0';
			
			RingManagerMsg(server->ringmanager, 1, buffer);
			break;
		}
		case (CLOSESTREAM):
			RingManagerMsg(server->ringmanager, 1, "\n");
			break;	
		case(HASH):
			printf("hash: %i\n", hash(RequestGetArg(request, 1)));
			break;
		case(SEND):
		{
			char buffer[100];
			fgets(buffer, 100, stdin);
			
			RingManagerMsg(server->ringmanager, 1, buffer);
			break;
		}
		case(EXIT):
			server->shutdown = 1;
			break;
		case(CHECK):
		{			
			int id = atoi(RequestGetArg(request, 1));
			
			if(RingManagerCheck(server->ringmanager, id)){
				printf("It's ours");
			} else {
				printf("No luck here");
			}
			
			fflush(stdout);
			break;
		}
		default:
			printf("Comando não reconhecido\n");
			break;
	}
	
	return 0;
}

void ServerSetIP(Server * server, char* ip){
	strcpy(server->ip,ip);
}

int hash(char *str)
{
    int h = 0;
    while (*str)
       h = h << 1 ^ *str++;
    return h;
}
