#include "UIManager.h"

int UIManagerArm(fd_set *rfds, int * maxfd){
    
    FD_SET(0, rfds);

    return 1;
}

int UIManagerReq(fd_set *rfds, Request * request){ /*Maybe fix to read pipelined commands*/
    
    int n;
    char buffer[128];
    
    if(!FD_ISSET(0,rfds) || ( n = read(0, buffer, 128)) == 0) return 0;
    FD_CLR(0, rfds);
    
    if(n == -1) exit(1);                /*ERROR HANDLING PLZ DO SMTHG EVENTUALLY*/
        
    buffer[n]='\0';
    RequestParseString(request, buffer);
    RequestAddFD(request, 0);
    
    return 1;
}
