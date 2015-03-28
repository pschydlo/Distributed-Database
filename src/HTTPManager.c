#include "HTTPManager.h"

struct HTTPManager{
    int pfd;
    int fd;
    int indexfd;
};

HTTPManager * HTTPManagerCreate(){
    return (HTTPManager*)malloc(sizeof(HTTPManager));
}

int HTTPManagerStart(HTTPManager * httpmanager, int port){
    httpmanager->pfd = TCPSocketCreate();
    httpmanager->indexfd = open("index.http", O_RDONLY);
    
    TCPSocketBind(httpmanager->pfd, port);
    if(TCPSocketListen(httpmanager->pfd)==-1) exit(1);

    return 0;
}

int HTTPManagerArm(HTTPManager * httpmanager, fd_set * rfds, int * maxfd ){
    
    FD_SET(httpmanager->pfd, rfds);
    if(httpmanager->pfd > *maxfd) *maxfd = httpmanager->pfd;
    
    return 0;
}

int HTTPManagerReq(HTTPManager * httpmanager, fd_set * rfds, Request * request){
    if(FD_ISSET(httpmanager->pfd,rfds)){
        struct stat stat_buf;
        
        fstat(httpmanager->indexfd, &stat_buf);

        /* copy file using sendfile */
        off_t offset = 0;
        
        int newfd = TCPSocketAccept(httpmanager->pfd);
        
        int rc = sendfile (newfd, httpmanager->indexfd, &offset, stat_buf.st_size);
        if (rc == -1) {
            printf("Unable to send file");
            fflush(stdout);
        }
        close(newfd);
    };
    return 0;
}