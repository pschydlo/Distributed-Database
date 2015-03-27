#include "RoutingTable.h"

struct Entry{
    int values[8];
    int last;
};

struct RoutingTable{
    Entry ** entries;
};

RoutingTable * RoutingTableCreate(int size){
    int i;
    RoutingTable * routingtable = malloc(sizeof(RoutingTable));
    routingtable->entries = malloc(sizeof(Entry*)*size);
    
    for(i = 0; i < size; i++){
        routingtable->entries[i] = malloc(sizeof(Entry));
        routingtable->entries[i]->last = 0;
    };
    
    return routingtable;
};

void RoutingTablePush(RoutingTable * routingtable, int key, int value){
    routingtable->entries[key]->last++;
    routingtable->entries[key]->values[(routingtable->entries[key])->last] = value;
};

int RoutingTablePop(RoutingTable * routingtable, int key){
    return (routingtable->entries[key]->values[(routingtable->entries[key])->last--]);
};

void RoutingTableDestroy(RoutingTable * routingtable){
};
