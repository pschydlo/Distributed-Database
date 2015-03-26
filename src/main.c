#include <stdlib.h>
#include "server.h"

#include <sys/types.h>
#include <ifaddrs.h>

void getIPAddr(char * ip);

int main(int argc, char **argv){
    Server * server;
    
    char ip[16];

    getIPAddr(ip);
	/*ProcArg(argc, argv); How will it return two ints and a string to give to ServerInit?*/
    printf("Our IP is now: %s\n\n", ip);/*We need to make it exit from lack of ports before it asks IP, plz*/

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

/*int ProcArg(int argc, char ** argv){
    
    char * ringPort = NULL;
    char * bootIP   = NULL;
    char * bootPort = NULL;
    int i, opt;

    opterr = 0;
    while ((opt = getopt(argc, argv, "t:i:p:")) != -1){
        switch (opt){
            case 't':
                ringPort = optarg;
                break;
            case 'i':
                bootIP = optarg;
                break;
            case 'p':
                bootPort = optarg;
                break;
            case '?':
                if (optopt == 't')
                    fprintf(stderr, "Opcao -%c requer argumento.\n", optopt);
                if (optopt == 'i')
                    fprintf(stderr, "Opcao -%c requer argumento.\n", optopt);
                if (optopt == 'p')
                    fprintf(stderr, "Opcao -%c requer argumento.\n", optopt);
                else if(isprint(optopt))
                    fprintf(stderr, "Opcao desconhecida '-%c'. Sintaxe: ddt [-t ringport] [-i bootIP] [-p bootport]\n", optopt);
                else
                    fprintf(stderr, "Caracter de opcao desconhecido '\\x%x'.\n", optopt);
                return 0;
            default:
                return 0;
        }
  }
  
    printf("tvalue = %s, ivalue = %s, pvalue = %s\n", ringPort, bootIP, bootPort);

    for (i = optind; i < argc; i++)
        fprintf (stderr, "Argumento invalido %s\n", argv[i]);
    
    if(ringPort != NULL)    server->TCPport = atoi(ringPort);
    if(bootIP   != NULL)    UDPManagerSetIP(server->udpmanager, bootIP);
    if(bootPort != NULL)    UDPManagerSetPort(server->udpmanager, atoi(bootPort));

    return 0;   
}*/
