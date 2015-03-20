#include "UImanager.h"

int UImanagerParse(Request * request, char * buffer){
	
	int n    = 0;
	int start = 0;
	
	while(buffer[n]!='\n' && buffer[n]!='\0'){
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
			UImanagerParse(request, buffer);
			
			return 1;
		}
	}
	
	/*printf("argc=%i\n", UImanagerParse(buffer, args, argc));*/
	
	return 0;
}
