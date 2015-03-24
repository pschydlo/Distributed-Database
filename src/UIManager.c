#include "UIManager.h"

int UIManagerParse(Request * request, char * buffer){
	int n, start = 0;
	RequestReset(request);
	
	for(n = 0; buffer[n] != '\0'; n++){
		if(buffer[n] != ' ' && buffer[n]!= '\n') continue;
		
		RequestPushArg(request, buffer + start, n - start);
		start = n + 1;
	}
	
	return 0;
}

int UIManagerArm(fd_set *rfds, int * maxfd){
	
	FD_SET(0, rfds);

	return 1;
}

int UIManagerReq(fd_set *rfds, Request * request){ /*Maybe fix to read pipelined commands*/
	
	int n;
	char buffer[128];
	
	if(!FD_ISSET(0,rfds) || ( n = read(0, buffer, 128)) == 0) return 0;
	FD_CLR(0, rfds);
	
	if(n == -1) exit(1);				/*ERROR HANDLING PLZ DO SMTHG EVENTUALLY*/
		
	buffer[n]='\0';
	UIManagerParse(request, buffer);
	
	return 1;
}
