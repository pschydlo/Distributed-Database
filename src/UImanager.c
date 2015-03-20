#include "UImanager.h"

int UImanagerParse(Request * request, char * buffer){
	int n, start = 0;
	RequestReset(request);
	
	for(n = 0; buffer[n] != '\0'; n++){
		if(buffer[n] != ' ' && buffer[n]!= '\n') continue;
		
		RequestPushArg(request, buffer + start, n - start);
		start = n + 1;
	}
	
	return 0;
}

int UImanagerArm(fd_set *rfds, int * maxfd){
	
	FD_SET(0, rfds);

	return 1;
}

int UImanagerReq(fd_set *rfds, Request * request){ /*Maybe fix to read pipelined commands*/
	
	int n;
	char buffer[128];
	
	if(!FD_ISSET(0,rfds) || ( n = read(0, buffer, 128)) == 0) return 0;
	
	if(n == -1) exit(1);				/*ERROR HANDLING PLZ DO SMTHG EVENTUALLY*/
		
	buffer[n]='\0';
	UImanagerParse(request, buffer);
		
	return 1;
}
