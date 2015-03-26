#include <stdlib.h>
#include "server.h"

#include <sys/types.h>
#include <ifaddrs.h>

void getIPAddr(char * ip);

int main(int argc, char **argv){
    Server * server;
    
    char ip[16];

    getIPAddr(ip);

    printf("Our IP is now: %s\n\n", ip);

    server = ServerInit(argc, argv, ip); /*Maybe process args in main, but looking good for now*/

    ServerStart(server);
    ServerStop(server);

    exit(0);
}


void getIPAddr(char * ip){
    struct ifaddrs *addrs, *ifa;
    int n, i=0;
    char ips[4][17];

    printf("\n");

    getifaddrs(&addrs);
    ifa = addrs;

    for (ifa = addrs; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family==AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *) ifa->ifa_addr;
            strcpy(ips[i],inet_ntoa(sa->sin_addr));
            printf("(%d) %-11s: %s\n", i, ifa->ifa_name, ips[i]);
            i++;
        }
    }

    printf("Which network interface do you wish to use?:");
    scanf("%i", &n);

    strcpy(ip, ips[n]);

    freeifaddrs(addrs);  
}

