#include "UImanager.h"

int UImanagerArm(fd_set *rfds, int * maxfd){
	
	FD_SET(0, rfds);

	return 1;
}

int UImanagerReq(fd_set *rfds, char * buffer){
	int n;
	
	if(FD_ISSET(0,rfds)){
		if((n=read(0,buffer,128))!=0){
			if(n==-1)exit(1);				/*ERROR HANDLING PLZ DO SMTHG EVENTUALLY*/
			buffer[n]='\0';
			return 1;
		}
	}
	
	return 0;
}
