#include <stdlib.h>

typedef struct RoutingTable RoutingTable;
typedef struct Entry Entry;

RoutingTable * RoutingTableCreate(int size);
void RoutingTablePush(RoutingTable * routingtable, int key, int value);
int RoutingTablePop(RoutingTable * routingtable, int key);
void RoutingTableDestroy(RoutingTable *routingtable);
