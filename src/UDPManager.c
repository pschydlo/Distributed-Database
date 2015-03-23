#include "UDPManager.h"

struct UDPManager{
	char * ip;
	int port, fd;
	struct sockaddr_in * addr;
	
};

int UDPManagerSetIP(UDPManager * udpmanager, char * bootIP){
	udpmanager->ip = bootIP;		/*Not sure if this is correct way*/
	return 0;
}

int UDPManagerSetPort(UDPManager * udpmanager, int bootPort){
	udpmanager->port = bootPort;
	return 0;
}

int UDPManagerJoin(UDPManager * udpmanager, int ring){
	/*strcat stuff here to join REG*/
	if(sendto(udpmanager->fd, "REG 9 0 1.2.3.4 1234", 20, 0, (struct sockaddr*)udpmanager->addr, sizeof(struct sockaddr)) == -1) exit(1);
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
	udpmanager->addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));	/*if you really want to, we can typedef this as sockaddr_in*/
	udpmanager->ip   = "193.136.138.142";	/*ip de tejo.tecnico.ulisboa.pt*/
	udpmanager->port = 58000;
	return udpmanager;
}

void UDPManagerStop ( UDPManager * udpmanager){
  free(udpmanager->addr);
}

int UDPManagerReq(UDPManager * udpmanager, fd_set * rfds, Request * request){

	int n = 0;
	
	char buffer[128];
	
	if(!FD_ISSET(udpmanager->fd, rfds)) return 0;
	
	socklen_t addrlen = sizeof(struct sockaddr_in);

	n = recvfrom(udpmanager->fd, buffer, 128, 0, (struct sockaddr*)udpmanager->addr, &addrlen);
	if(n == -1) exit(1);
	
	buffer[n] = '\n';
	buffer[n+1] = '\0';
	
	RequestParseString(request, buffer);
	
	return 1;
}
