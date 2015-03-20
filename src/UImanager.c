#include "UImanager.h"

int UImanagerParse(char * buffer, Request * request){
	
	int n    = 0;
	int start = 0;
	
	while(buffer[n]!='\n'){
		if(buffer[n] == ' '){
			RequestPushArg(request, buffer + start, n - start);
			start = n + 1;
		}
		printf("%c", buffer[n]);
		n++;
	}

	//0->n
	//PushArg(request, memstart, length);

	return 0;
}

int UImanagerArm(fd_set *rfds, int * maxfd){
	
	FD_SET(0, rfds);

	return 1;
}

int UImanagerReq(fd_set *rfds, Request * request){ /*Maybe fix to read pipelined commands*/
	
	int n;
	char buffer[128];
	
	if(FD_ISSET(0,rfds)){
		if((n=read(0,buffer,128))!=0){
			if(n==-1)exit(1);				/*ERROR HANDLING PLZ DO SMTHG EVENTUALLY*/
			
			buffer[n]='\0';
			RequestWrite(request, buffer);
			
			return 1;
		}
	}
	
	/*printf("argc=%i\n", UImanagerParse(buffer, args, argc));*/
	
	return 0;
}
