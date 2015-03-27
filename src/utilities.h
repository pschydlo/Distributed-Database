#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

struct Config {
    char extIP[16];
    int  extPort;

    char bootServerIP[16];
    int  bootServerPort;
    
    int  debug;
};

typedef struct Config Config;
    
Config * ConfigCreate();
void ConfigDestroy(Config * config);
    
void getIPAddr(Config * config);
int ProcArg(int argc, char ** argv, Config * config);