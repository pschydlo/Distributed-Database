#include "server.h"

#define max(A,B) ((A)>=(B)?(A):(B)) /*I think we don't even use this, we just do it manually in the while loop*/

struct Server{
	int isBoot;
	UDPManager  * udpmanager;
	TCPManager  * tcpmanager;
	RingManager * ringmanager;
	char ip[16];
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
	int i = 0;
	char * command;
	char buffer[100];
	
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
		if(count == 3){
      
			UDPManagerJoin(server->udpmanager, atoi(RequestGetArg(request, 1)), atoi(RequestGetArg(request, 2)));
      
			return 0;
		}else if(count == 6){
			
			int ring = atoi(RequestGetArg(request, 1));
			int id   = atoi(RequestGetArg(request, 2));
			int succiID    = atoi(RequestGetArg(request, 3));
			char * succiIP = RequestGetArg(request, 4); 
			int succiPort = atoi(RequestGetArg(request, 5)); 

			RingManagerConnect(server->ringmanager, ring, id, succiID, succiIP, succiPort);
			return 0;
		}	
	}else if(strcmp(command,"leave") == 0){
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
		return 0;
	}
	else if(strcmp(command,"show") == 0) RingManagerStatus(server->ringmanager);
  else if(strcmp(command,"rsp") == 0) RingManagerRsp(server->ringmanager, 0, 1, RingManagerId(server->ringmanager), server->ip, server->TCPport);
	else if(strcmp(command,"search") == 0){		/*Reminder: limit commands if user is not connect to ring*/
		if(count < 2) return 0;
    
		int search = atoi(RequestGetArg(request, 1));
		int id     = RingManagerId(server->ringmanager);
		
		if(RingManagerCheck(server->ringmanager, search)) printf("Yey, don't have to go far: %i, ip, port\n", id); /*Add variables for ip and port eventually*/
		else RingManagerQuery(server->ringmanager, id, search); /*Add int->string support eventually*/
		return 0;
	}else if(strcmp(RequestGetArg(request,0),"boops") == 0) RingManagerMsg(server->ringmanager, 0, "Boop\nBoop\n");/*Debugging boops*/
	else if(strcmp(RequestGetArg(request,0),"boopp") == 0) RingManagerMsg(server->ringmanager, 1, "Boop\nBoop\n");
	else if(strcmp(RequestGetArg(request,0),"part1") == 0) RingManagerMsg(server->ringmanager, 1, "Bo");
	else if(strcmp(RequestGetArg(request,0),"part2") == 0) RingManagerMsg(server->ringmanager, 1, "op\n");
	else if(strcmp(RequestGetArg(request,0),"send") == 0){
		fgets(buffer, 100, stdin);
		RingManagerMsg(server->ringmanager, 1, buffer);
		return 0;
	}else if(strcmp(RequestGetArg(request,0),"exit") == 0) server->shutdown = 1;
	else if(strcmp(RequestGetArg(request,0),"check") == 0){
	  if(RingManagerCheck(server->ringmanager, atoi(RequestGetArg(request, 1)))) printf("It's ours");
	  else printf("No luck here");
	  fflush(stdout);
	  return 0;
	} 
	else printf("Comando não reconhecido\n");
	
	return 1;
}

void ServerSetIP(Server * server, char* ip){
	strcpy(server->ip,ip);
}

unsigned long hash(unsigned char *str) /* djb2 algorithm */
{
	unsigned long hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}
