#include "Buffer.h"

struct Buffer{
    char buffer[10][128];
    int top;
    int bottom;
};
