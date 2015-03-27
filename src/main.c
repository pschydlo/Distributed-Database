#include <stdlib.h>
#include "server.h"
#include "utilities.h"

int main(int argc, char **argv){
    Server * server;
    Config * config;

    config = ConfigCreate();
    
    /* Populate config with enviroment settings */
    ProcArg(argc, argv, config);
    getIPAddr(config);

    /* Configure and start server*/
    server = ServerInit();
    //ServerSetBootServer(server, config->bootServerIP, config->bootServerPort);
    ServerStart(server, config->extIP, config->extPort);
    
    /* Free memory */
    ServerDestroy(server);
    ConfigDestroy(config);
    
    exit(0);
}