#include "UDPManager.h"

struct UDPManager{
	char ip[16];
	int ring, id;
	int port, fd, tcpfd;
	struct sockaddr_in * addr;
	
};

int UDPManagerID(UDPManager * udpmanager){
	return udpmanager->id;
}

int UDPManagerRing(UDPManager * udpmanager){
	return udpmanager->ring;
}

int UDPManagerMsg(UDPManager * udpmanager, char * buffer){
	return sendto(udpmanager->fd, buffer, strlen(buffer), 0, (struct sockaddr*)udpmanager->addr, sizeof(struct sockaddr));
}

int UDPManagerJoin(UDPManager * udpmanager, int ring, int id){
	char buffer[128];
	udpmanager->ring = ring;
	udpmanager->id   = id;
	sprintf(buffer, "BQRY %d", ring);
	return UDPManagerMsg(udpmanager, buffer);
}

int UDPManagerSetIP(UDPManager * udpmanager, char * bootIP){
	strcpy(udpmanager->ip, bootIP);		/*Not sure if this is correct way*/
	return 0;
}

int UDPManagerSetPort(UDPManager * udpmanager, int bootPort){
	udpmanager->port = bootPort;
	return 0;
}

int UDPManagerReg(UDPManager * udpmanager, char * ip, int port){
	char buffer[128];
	sprintf(buffer, "REG %d %d %s %d", udpmanager->ring, udpmanager->id, ip, port);
	if(UDPManagerMsg(udpmanager, buffer) == -1) exit(1); /*Error handle better, please*/
	return 0;
}


int UDPManagerArm( UDPManager * udpmanager, fd_set * rfds, int * maxfd ){
	FD_SET(udpmanager->fd, rfds);
	if(udpmanager->fd > *maxfd) *maxfd = udpmanager->fd;
	return 0;
}

int UDPManagerCreate(UDPManager * udpmanager){
	
	udpmanager->fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(udpmanager->fd == -1) exit(1);
	memset((void*)udpmanager->addr,(int)'\0',sizeof(struct sockaddr_in));
	(udpmanager->addr->sin_family)=AF_INET;
	udpmanager->addr->sin_port=htons(udpmanager->port);
	inet_pton(AF_INET, udpmanager->ip, &(udpmanager->addr->sin_addr));
	
	return 0;
}

UDPManager * UDPManagerInit(){
	UDPManager * udpmanager = (UDPManager*)malloc(sizeof(UDPManager));
	memset(udpmanager, 0, sizeof(UDPManager));
	
	udpmanager->addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));	/*if you really want to, we can typedef this as sockaddr_in*/
	memset(udpmanager->addr, 0, sizeof(struct sockaddr_in));
	
	strcpy(udpmanager->ip, "193.136.138.142");	/*ip de tejo.tecnico.ulisboa.pt*/
	udpmanager->port = 58000;
	return udpmanager;
}

void UDPManagerStop ( UDPManager * udpmanager){
  free(udpmanager->addr);
	close(udpmanager->fd);
	free(udpmanager);
}

int UDPManagerReq(UDPManager * udpmanager, fd_set * rfds, Request * request){

	int n = 0;
	
	char buffer[128];
	
	if(!FD_ISSET(udpmanager->fd, rfds)) return 0;
	FD_CLR(udpmanager->fd, rfds);
	
	socklen_t addrlen = sizeof(struct sockaddr_in);

	n = recvfrom(udpmanager->fd, buffer, 128, 0, (struct sockaddr*)udpmanager->addr, &addrlen);
	if(n == -1) exit(1);
	
	buffer[n] = '\n';
	buffer[n+1] = '\0';
	
	RequestParseString(request, buffer);
	
	return 1;
}
