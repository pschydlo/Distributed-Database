#include "UDPManager.h"

struct UDPManager{
	char * ip;
	int port;
};

int UDPManagerSetIP(UDPManager * udpmanager, char * bootIP){
	strcpy(udpmanager->ip, bootIP);
	return 0;
}

int UDPManagerSetPort(UDPManager * udpmanager, int bootPort){
	udpmanager->port = bootPort;
	return 0;
}

int UDPManagerJoin(UDPManager * udpmanager, int ring){
	
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd == -1) exit(1);
	
	struct sockaddr_in addr;

	memset((void*)&addr,(int)'\0',sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(udpmanager->port);
	inet_pton(AF_INET, udpmanager->ip, &(addr.sin_addr));

}

UDPManager * UDPManagerInit(){
	UDPManager * udpmanager = (UDPManager*)malloc(sizeof(UDPManager));
	udpmanager->ip   = "193.136.138.142";
	udpmanager->port = 58000;
	return udpmanager;
}
