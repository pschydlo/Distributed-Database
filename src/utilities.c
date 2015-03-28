#include "utilities.h"

Config * ConfigCreate(){
    Config * config = (Config*)malloc(sizeof(Config));
    memset(config, 0, sizeof(Config));
    return config;
}

void ConfigDestroy(Config * config){
    free(config);
}

void getIPAddr(Config * config){
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

    strcpy(config->extIP, ips[n]);

    freeifaddrs(addrs);  
}

int ProcArg(int argc, char ** argv, Config * config){
    
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
    
    if(ringPort != NULL) config->extPort = atoi(ringPort);
    else config->extPort = 9000;
    
    if(bootIP   != NULL) strcpy(config->bootServerIP, bootIP);
    else strcpy(config->bootServerIP, "193.136.138.142");
    
    if(bootPort != NULL) config->bootServerPort = atoi(bootPort);
    else config->bootServerPort = 58000;
    
    return 0;   

}
